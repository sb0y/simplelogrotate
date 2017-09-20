#include <iostream>

#include "logrotate.hpp"

int main( int /*argc*/, char ** /*argv*/ )
{
	try {
		// number of logs, file size in bytes, logging directory
		logrotate::logrotate log{ 5, 10, "../data" };
		log.Write( "test" );
		log.DateWrite( "lorem" );
	} catch( const std::exception &e ) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}