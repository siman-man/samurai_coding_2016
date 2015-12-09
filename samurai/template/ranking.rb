require 'erb'

contents =<<-EOS
<!DOCTYPE html>
<html lang="ja">
  <head>
    <title>Ranking</title>
  </head>
  <body>
    <p>Hello World</p>
  </body>
</html>
EOS

erb = ERB.new(contents,nil,'%')

File.open('ranking.html','w') do |page|
  page.puts(erb.result(binding))
end
