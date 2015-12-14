require 'sqlite3'

db = SQLite3::Database.open('./db/ranking.db')

sql = <<-SQL
CREATE TABLE ranking_table (
  name text,
  udemae text,
  total_nuri_point integer,
  play_count integer,
  total_kill_count integer,
  total_die_count integer
);
SQL

begin
  # テーブルを再作成
  db.execute(sql)
rescue => ex
  puts ex.message
ensure
  db.close
end
