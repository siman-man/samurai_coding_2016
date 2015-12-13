@filename = "siman"
@ainame = 'ultra_greedy_man'

task :default do
  `g++ -W -Wall -Wno-sign-compare -O2 -o #{@ainame} #{@filename}.cpp`
end

task :release do
  `g++ -W -Wall -Wno-sign-compare -O2 -o #{@ainame} #{@filename}.cpp`
  `mv #{@ainame} samurai/players`
end

