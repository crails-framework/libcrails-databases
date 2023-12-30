#include "databases.hpp"

using namespace std;
using namespace Crails;

thread_local Crails::Databases Crails::databases;

Databases::Database::~Database()
{
}

void Databases::cleanup_databases()
{
  for (auto it = databases.begin() ; it != databases.end() ; ++it)
    delete *it;
  databases.clear();
}

void Databases::cleanup_database(Database& database)
{
  auto it = std::find(databases.begin(), databases.end(), &database);

  if (it != databases.end())
  {
    delete *it;
    databases.erase(it);
  }
  else
    throw boost_ext::out_of_range("Database isn't currently handled by this Databases Manager");
}

Databases::Database* Databases::get_database_from_name(const std::string& key)
{
  for (auto it = databases.begin() ; it != databases.end() ; ++it)
  {
    if (**it == key)
      return (*it);
  }
  return (0);
}

const Databases::DatabaseSettings& Databases::get_database_settings_for(const std::string& key) const
{
  Settings&                    settings = Settings::singleton::require();
  Settings::const_iterator     environment_settings = settings.find(Crails::environment);
  DatabasesMap::const_iterator database_settings;

  if (environment_settings == settings.end())
    throw Databases::Exception("Database configuration not found for environment '" + Crails::environment_name(Crails::environment) + '\'');
  database_settings = environment_settings->second.find(key);
  if (database_settings == environment_settings->second.end())
    throw Databases::Exception("Database configuration not found for '" + key + "'");
  return database_settings->second;
}
