require 'sqlite3'

db = SQLite3::Database.open('./db/ranking.db')

begin
  count = db.get_first_value("select count(*) from ranking_table where name = 'siman'")
  p count
ensure
  db.close
end
