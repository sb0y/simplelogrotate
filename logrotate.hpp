#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <utility>

namespace logrotate {

class logrotate {
public:
	logrotate( int vNumber, int vMaxSize, const std::string &path );
	~logrotate();

	using fileList_t = std::vector< std::pair< std::string, std::time_t > >;

	// -Weffc++
	logrotate( const logrotate& ) = delete;
	logrotate& operator=( const logrotate& ) = delete;
	logrotate( logrotate&& ) noexcept = delete;
	logrotate& operator=( logrotate&& ) noexcept = delete;

	void Write( const std::string &vText );
	void DateWrite( const std::string &vText );
private:
	std::string _path;
	int _number, _maxSize;
	std::ofstream _currentFile;
	std::string _currentFilePath;

	enum class FILE_ERR {
		BAD_SIZE,
		NO_EXISTS,
		NO_ERROR
	};

	fileList_t getDirList( const std::string &path );
	void writeImpl( const std::string &text );
	std::string toFilePath( int number );
	std::string toFilePath( const std::string &fname );
	std::string toFileName( int number );
	FILE_ERR isFileOk( const std::string &fpath );
	bool remove( const std::string &fpath );
	bool createAndWriteHelper( std::size_t number );
	bool rotate( fileList_t &dirList );
	bool renameAllFiles( fileList_t &dirList );
};

} // namespace logrotate