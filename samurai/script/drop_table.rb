require 'sqlite3'

db = SQLite3::Database.new('./db/ranking.db')

begin
  db.execute('DROP TABLE ranking_table')
rescue => ex
  puts ex.message
ensure
  db.close
end
