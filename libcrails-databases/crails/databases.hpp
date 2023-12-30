#ifndef  DATABASES_HPP
# define DATABASES_HPP

# include <map>
# include <vector>
# include <crails/datatree.hpp>
# include <crails/environment.hpp>
# include <crails/utils/backtrace.hpp>
# include <crails/utils/singleton.hpp>

# define CRAILS_DATABASE(type,database) \
  Crails::databases.get_database<type::Database>(database)

# define CRAILS_DATABASE_FROM_SETTINGS(type,database,settings) \
  Crails::databases.get_database<type::Database>(database,settings)

namespace Crails
{
  class Databases
  {
  public:
    typedef std::map<std::string, boost::any> DatabaseSettings;
    typedef std::map<std::string, DatabaseSettings> DatabasesMap;
    typedef std::map<Environment, DatabasesMap> SettingsMap;

    class Settings : public SettingsMap
    {
      SINGLETON(Settings)
    public:
      Settings(const SettingsMap& data) : SettingsMap(data) {}
      Settings() {}
    };

    class Database
    {
      friend class Databases;
    public:
      Database(const std::string& type) : type(type) {}
      virtual ~Database();

      bool               operator==(const std::string& name) const { return this->name == name; }
      const std::string& get_type() const { return type; }
      const std::string& get_name() const { return name; }
      virtual void       connect(void) = 0;

    protected:
      std::string        name;
      std::string        type;
    };

    typedef std::vector<Database*> DatabaseList;

    struct Exception : public boost_ext::exception
    {
      Exception(const std::string& message) : message(message) {}

      const char* what() const throw() { return (message.c_str()); }
      std::string message;
    };

    Databases()
    {
    }

    ~Databases()
    {
      cleanup_databases();
    }

    void cleanup_databases();

    void cleanup_database(Database&);

    Database* get_database_from_name(const std::string& key);

    template<typename TYPE>
    Database* initialize_database(const std::string& key, const Crails::Databases::DatabaseSettings& settings)
    {
      TYPE*     database = new TYPE(settings);
      Database* abstract_database = reinterpret_cast<Database*>(database);

      abstract_database->name = key;
      databases.push_back(abstract_database);
      return abstract_database;
    }

    template<typename TYPE>
    TYPE& get_database(const std::string& key, const Crails::Databases::DatabaseSettings& settings)
    {
      Database* db = get_database_from_name(key);

      if (!db)
        db = initialize_database<TYPE>(key, settings);
      if (db->get_type() != TYPE::ClassType())
        throw Databases::Exception("Expected type '" + TYPE::ClassType() + "', got '" + db->get_type() + '\'');
      db->connect();
      return (*(reinterpret_cast<TYPE*>(db)));
    }

    template<typename TYPE>
    TYPE& get_database(const std::string& key)
    {
      return get_database<TYPE>(key, get_database_settings_for(key));
    }

    const Crails::Databases::DatabaseSettings& get_database_settings_for(const std::string& key) const;

  private:
    DatabaseList databases;
  };

  extern thread_local Databases databases;
}

#endif
