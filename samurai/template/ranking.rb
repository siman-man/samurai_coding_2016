require 'erb'
require 'sqlite3'

contents =<<-EOS
<!DOCTYPE html>
<html lang="ja">
  <head>
    <title>Ranking</title>
  </head>
  <body>
    <p>Hello World</p>
    <ul>
      <% ranking_list.each do |list| %>
        <li><%= list %></li>
      <% end %>
    </ul>
  </body>
</html>
EOS

begin
  erb = ERB.new(contents, nil, '%')
  db = SQLite3::Database.open('./db/ranking.db')

  ranking_list = []

  db.execute("SELECT * FROM ranking_table") do |row|
    ranking_list << "Name: #{row[0]}, udemae: #{row[1]}, nuri_point: #{row[2]}, play_count: #{row[3]}"
  end

  html_path = File.expand_path('../ranking.html', __FILE__)

  File.open(html_path,'w') do |page|
    page.puts(erb.result(binding))
  end
ensure
  db && db.close
end
