#include <iomanip>
#include <iostream>
#include <algorithm>
#include <chrono>

#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logrotate.hpp"

#define LOG_NAME "app"

namespace logrotate 
{

logrotate::logrotate( int vNumber, int vMaxSize, const std::string &path )
: _path{ path }
, _number{ vNumber }
, _maxSize{ vMaxSize }
, _currentFile{}
, _currentFilePath{}
{
	if( _number < 1 ) {
		throw std::runtime_error{ "`vNumber` must be greater than 0" };
	}

	if( _maxSize < 1 ) {
		throw std::runtime_error{ "`vMaxSize` must be greater than 0" };
	}
}

logrotate::~logrotate() = default;

void logrotate::Write( const std::string &vText )
{
	writeImpl( vText );
}

void logrotate::DateWrite( const std::string &vText )
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t( now );
	std::stringstream ss;
	ss << std::put_time( std::localtime( &in_time_t ), "[ %T %D ] " );
	ss << vText;
	
	writeImpl( ss.str() );
}

logrotate::fileList_t logrotate::getDirList( const std::string &path )
{
	fileList_t list;
	list.reserve( _number );

	if( auto dir = opendir( path.c_str() ) ) {
		while( auto f = readdir( dir ) ) {
			if( !f->d_name || f->d_name[ 0 ] == '.' )
				continue; // skip everything that starts with a dot

			struct stat sb = {};
			stat( toFilePath( f->d_name ).c_str(), &sb );

			// something really bad, like std::bad_alloc
			try {
				list.push_back( std::make_pair( f->d_name, sb.st_mtime ) );
			} catch( const std::exception& ) {
				closedir( dir );
				return list;
			}
		}

		closedir( dir );
	}

	return list;
}

std::string logrotate::toFileName( int number )
{
	return std::string{ LOG_NAME "_" } + std::to_string( number ) + ".log";
}

std::string logrotate::toFilePath( int number )
{
	std::stringstream ss;
	ss << _path;
	ss << "/";
	ss << LOG_NAME;
	ss << "_";
	ss << number;
	ss << ".log";

	return ss.str();
}

std::string logrotate::toFilePath( const std::string &fname )
{
	return _path + "/" + fname;
}

logrotate::FILE_ERR logrotate::isFileOk( const std::string &fpath )
{
	struct stat sb = {};
	if( stat( fpath.c_str(), &sb ) == -1 ) {
		return FILE_ERR::NO_EXISTS; 
	}

	if( static_cast< int >( sb.st_size ) >= _maxSize ) {
		return FILE_ERR::BAD_SIZE;
	}

	return FILE_ERR::NO_ERROR;
}

bool logrotate::remove( const std::string &fname )
{
	std::string fpath = _path + "/" + fname;
	return unlink( fpath.c_str() ) != -1;
}

bool logrotate::createAndWriteHelper( std::size_t number )
{
	_currentFilePath = toFilePath( number );
	_currentFile.open( _currentFilePath, std::ios::app );
	if( !_currentFile.bad() ) {
		return true;
	}

	return false;
}

void printAll( const logrotate::fileList_t &l )
{
	std::cout << "ALL FILES PRINT: " << std::endl;

	for( const auto &f : l ) {
		std::cout << f.first << " : " << f.second << std::endl;
	}
}

bool logrotate::renameAllFiles( fileList_t &dirList )
{	
	for( int i = static_cast< int >( dirList.size() ) - 1, j = 0; i >= 0; --i, ++j ) {
		std::string file  = toFileName( j ),
					  dst = toFilePath( file ),
					  src = toFilePath( dirList[ i ].first );

		if( std::rename( src.c_str(), dst.c_str() ) == -1 ) {
			std::cerr << "Failed to rename file! Check permissions!" << std::endl;
			return false;
		}

		dirList[ i ].first = std::move( file );
	}

	return true;
}

bool logrotate::rotate( fileList_t &dirList )
{
	for( const auto &f : dirList ) {
		std::string const fp = toFilePath( f.first );

		switch( isFileOk( fp ) ) {

			//case FILE_ERR::NO_EXISTS:
			case FILE_ERR::BAD_SIZE:

				if( &f == &dirList.back() ) { // this our last iteration					
					if( _number > static_cast< int >( dirList.size() ) ) {
						std::cout << "Creating new log file ..." << std::endl;
						renameAllFiles( dirList );
						return createAndWriteHelper( dirList.size() );
					} else {
						std::string el = dirList.back().first;
						if( remove( el ) ) {
							std::cout << "File " << el << " deleted" << std::endl;
							dirList.pop_back();
							if( renameAllFiles( dirList ) ) {
								return createAndWriteHelper( dirList.size() );
							} else {
								return false;
							}
						}
					}
				}
			break;
	
			case FILE_ERR::NO_ERROR:
				_currentFile.open( fp, std::ios::app );
				if( !_currentFile.is_open() ) {
					throw std::runtime_error{ "Failed to open file " + fp };
				}
				std::cout << "Writing to existing file: " << fp << std::endl;
					
				return true;
			break;

			default:
				std::cerr << "Warning! Unhandled action!" << std::endl;
				return false;
		}
	}

	return false;
}

void logrotate::writeImpl( const std::string &text )
{
	fileList_t dirList = getDirList( _path );
	std::sort( dirList.begin(), dirList.end(), []( const auto &p1, const auto &p2 ){ return p1.second > p2.second; } );

	if( dirList.empty() ) {
		if( createAndWriteHelper( dirList.size() ) ) {
			_currentFile << text << std::endl;
		}
	} else if( _currentFile.is_open() && !_currentFile.bad() && isFileOk( _currentFilePath ) == FILE_ERR::NO_ERROR ) {
		_currentFile << text << std::endl;
	} else {
		if( rotate( dirList ) ) {
			_currentFile << text << std::endl;
		}
	}
}

} // namespace logrotate