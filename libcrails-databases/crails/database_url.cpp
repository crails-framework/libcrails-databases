#include "database_url.hpp"

using namespace std;
using namespace Crails;

void DatabaseUrl::initialize(const std::string_view url)
{
  if (url.length() > 0)
  {
    try
    {
      type     = string(substr(url, get_protocol_range(url)));
      hostname = string(substr(url, get_hostname_range(url)));
      username = string(substr(url, get_username_range(url)));
      password = string(substr(url, get_password_range(url)));
      port     = boost::lexical_cast<unsigned short>(substr(url, get_port_range(url)));
      database_name = string(substr(url, get_database_name_range(url)));
    }
    catch (std::exception& e)
    {
      logger << "Failed to read database url string " << url << Logger::endl;
      throw e;
    }
  }
}

string DatabaseUrl::to_string() const
{
  stringstream stream;

  stream << type << "://";
  if (username.length())
  {
    stream << username;
    if (password.length())
      stream << ':' << password;
    stream << '@';
  }
  stream << hostname;
  if (port != 0)
    stream << ':' << port;
  if (database_name.length())
    stream << '/' << database_name;
  return stream.str();
}

string_view DatabaseUrl::substr(const string_view url, pair<int, int> range)
{
  return url.substr(range.first, range.second - range.first);
}
