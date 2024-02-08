#ifndef  DATABASE_URL_HPP
# define DATABASE_URL_HPP

# include <boost/lexical_cast.hpp>
# include <crails/logger.hpp>
# include <string_view>

namespace Crails
{
  struct DatabaseUrl
  {
    DatabaseUrl() {}
    DatabaseUrl(const char* str)
    {
      if (str)
        initialize(str);
    }

    void initialize(const std::string_view url);
    std::string to_string() const;
    operator std::string() const { return to_string(); }

    std::string  type, hostname, username, password, database_name;
    unsigned int port;

  private:
    std::string_view substr(const std::string_view url, std::pair<int,int> range);

    std::pair<int, int> get_protocol_range(const std::string_view url)
    {
      return { 0, url.find("://") };
    }

    std::pair<int, int> get_username_range(const std::string_view url)
    {
      int start_name = get_protocol_range(url).second + 3;
      return { start_name, url.find(":", start_name) };
    }

    std::pair<int, int> get_password_range(const std::string_view url)
    {
      int start_password = get_username_range(url).second + 1;
      return { start_password, url.find("@", start_password) };
    }

    std::pair<int, int> get_hostname_range(const std::string_view url)
    {
      int start_hostname = get_password_range(url).second + 1;
      return { start_hostname, url.find(":", start_hostname) };
    }

    std::pair<int, int> get_port_range(const std::string_view url)
    {
      int start_port = get_hostname_range(url).second + 1;
      return { start_port, url.find("/", start_port) };
    }

    std::pair<int, int> get_database_name_range(const std::string_view url)
    {
      int start_database_name = get_port_range(url).second + 1;
      return { start_database_name, url.length() };
    }
  };
}

#endif
