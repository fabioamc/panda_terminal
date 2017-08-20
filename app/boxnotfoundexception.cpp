#include "boxnotfoundexception.h"
#include <string>
BoxNotFoundException::BoxNotFoundException( std::string message ) : std::runtime_error( message ) {
}
