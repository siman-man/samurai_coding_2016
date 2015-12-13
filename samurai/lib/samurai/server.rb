module SamurAI
	class Server
		attr_reader :max_turn, :width, :height, :field, :healing_time,
                :kyokan_list, :player_list, :current_turn, :current_player,
                :player_order, :db, :nuri_point

    # 参加プレイヤーの最大人数
    MAX_PLAYER_NUM = 6

    # ウデマエによるポイントの上昇率
    UDEMAE_UP_RATE = {
      'C-' => 7, 'C' => 6, 'C+' => 5,
      'B-' => 5, 'B' => 4, 'B+' => 4,
      'A-' => 4, 'A' => 4, 'A+' => 4,
      'S-' => 3, 'S' => 2, 'S+' => 1
    }.freeze

    # ウデマエによるポイントの下降率
    UDEMAE_DOWN_RATE = {
      'C-' => 1, 'C' => 2, 'C+' => 2,
      'B-' => 2, 'B' => 2, 'B+' => 2,
      'A-' => 2, 'A' => 2, 'A+' => 2,
      'S-' => 5, 'S' => 6, 'S+' => 7
    }.freeze

    # 次のランク
    NEXT_UDEMAE = {
      'C-' => 'C', 'C' => 'C+', 'C+' => 'B-',
      'B-' => 'B', 'B' => 'B+', 'B+' => 'A-',
      'A-' => 'A', 'A' => 'A+', 'A+' => 'S-',
      'S-' => 'S', 'S' => 'S+', 'S+' => 'SS'
    }.freeze

    # 前のランク
    PRED_UDEMAE = {
      'C-' => 'C-', 'C' => 'C-', 'C+' => 'C',
      'B-' => 'C+', 'B' => 'B-', 'B+' => 'B',
      'A-' => 'B+', 'A' => 'A-', 'A+' => 'A',
      'S-' => 'A+', 'S' => 'S-', 'S+' => 'S'
    }.freeze

    # プレイヤーのID
    A0 = 0
    A1 = 1
    A2 = 2
    B0 = 3
    B1 = 4
    B2 = 5

		def initialize
    end

    #
    # ゲーム情報の初期化を行う
    #   1. 最大ターンの決定
    #   2. フィールドの横幅を決める
    #   3. フィールドの縦幅を決める
    #   4. 治癒時間の決定
    #   5. 居館のリストを作成
    #   6. フィールドの状態を初期化
    #
    def init_game
      # データベースを開く
      open_db
      @max_turn = 120
      #@max_turn = 12 * [*1..84].sample
      @width = [*10..20].sample
      @height = [*10..20].sample
      @healing_time = [*12..48].sample
      @current_turn = 0

      # プレイヤーが行動する順序
      @player_order = [A0, B0, B1, A1, A2, B2, B0, A0, A1, B1, B2, A2].cycle

      create_field(width: width, height: height)
      init_kyokan_list
      init_player_list
    end

    #
    # フィールドの作成
    #
    def create_field(width:, height:)
      @field = SamurAI::Field.new(width: width, height: height)
    end

    #
    # ゲームを開始
    #
    def run
      begin
			  # ゲーム情報を初期化
			  init_game

        # プレイヤー情報を取得
        player_list.each do |player|
          player.load
				  player.input(first_input_params(id: player.id, group_id: player.group_id))
          player.response
        end

        # max_turnの数繰り返す
        max_turn.times do |turn|
          @current_turn = turn
          @current_player = player_list[player_order.next]
          #puts "current turn: #{turn}, player: #{current_player.id}"
          player_action
        end

        # 結果を集計 & 表示
        show_result
      ensure
        close_db
      end
    end

    #
    # プレイヤーが行う行動
    #
    def player_action
      clear_phase
      start_phase
      action_phase
      update_phase
    end

    #
    # ターンの一番最初にあるフェーズです。
    #   1. 各プレイヤーの体力を1回復します
    #   2. 各プレイヤーの視界情報を更新
    #
    def start_phase
      player_list.each do |player|
        player.cure
        player.view(field: field)
      end
    end

    #
    # ゲームの状態をクリアする
    #
    def clear_phase
      field.clear
    end

    #
    # ユーザが行動を行うフェーズです
    # ユーザから受け取った命令リストを元にゲームの盤面を更新します
    #
    def action_phase
			#puts "action phase =>"

      # プレイヤーに対して情報を送信
      current_player.input(input_params(current_player.id))

      # プレイヤーからの情報を受け取る
      output = current_player.response

      #puts "player #{current_player.name} = #{output.inspect}"

      # 命令を解析
      operation_list = parse_operation(output)

      # 命令を実行
      exec_operation(player: current_player, operation_list: operation_list)
    end

    #
    # ゲームの状態を更新する
    #   1. 盤面を調べて、攻撃されたマスに他のプレイヤーがいた場合は居館に戻して治療期間を設定する
    #
    def update_phase
			#puts "update phase =>"

      player_list.each do |player|
        next if current_player.id == player.id

        cell = field[player.y][player.x]

        if cell.attacked
          player.go_back_home
          player.cure_period = healing_time
        end
      end
    end

    #
    # 渡された命令を処理する。
    # コストをオーバーすると失敗
    # @params operation_list 命令リスト
    #
    def exec_operation(player:, operation_list:)
      cost = 0

      # 治療期間が終わっていない場合は全ての命令を無視
      if player.cure_period > 0
        return
      end

      operation_list.each do |operation|
        case operation
          # 占領
          when 1..4
            direct = operation - 1
            player.attack(direct: direct, field: field)
            cost += 2
          # 行動
          when 5..8
            direct = operation - 5
            player.move(direct: direct, field: field)
            cost += 4
          # 潜伏
          when 9
            player.hide(field: field)
            cost += 1
          # 顕現
          when 10
            player.show_up(field: field)
            cost += 1
          else
        end
      end
    end

    #
    # 最終結果
    #
    def show_result
      group_point = Hash.new(0)

			(0...height).each do |y|
        (0...width).each do |x|
          cell = field[y][x]

          if cell.owner != 8
            player_list[cell.owner].point += 1
            group_point[cell.owner_group] += 1
          end
        end
      end

      puts "Team A: #{group_point[0]} point, Team B: #{group_point[1]} point"

      if group_point[0] < group_point[1]
        puts 'Team 1 Win'
        update_ranking(winner: 1)
      elsif group_point[0] > group_point[1]
        puts 'Team 0 Win'
        update_ranking(winner: 0)
      else
        puts 'Draw'
      end

      puts "Personal Result:"

      player_list.each do |player|
        puts "Player #{player.name}: #{player.point} point"
      end

      player_list.each do |player|
        db.execute("select * from ranking_table where name = '#{player.name}'") do |row|
          puts "name: #{row[0]}, udemae: #{row[1]}, nuri_point: #{row[2]}, play_count: #{row[3]}"
        end
      end
    end

    #
    # ランクの更新を行う
    #
    def update_ranking(winner:)
      if winner == 0
        [0,1,2].each do |id|
          player = player_list[id]
          rank_up(name: player.name, udemae: player.udemae)
          update_total_nuri_point(name: player.name, total_nuri_point: player.total_nuri_point + player.point)
          update_play_count(name: player.name, play_count: player.play_count + 1)
        end

        [3,4,5].each do |id|
          player = player_list[id]
          rank_down(name: player.name, udemae: player.udemae)
          update_total_nuri_point(name: player.name, total_nuri_point: player.total_nuri_point + player.point)
          update_play_count(name: player.name, play_count: player.play_count + 1)
        end
      else
        [3,4,5].each do |id|
          player = player_list[id]
          rank_up(name: player.name, udemae: player.udemae)
          update_total_nuri_point(name: player.name, total_nuri_point: player.total_nuri_point + player.point)
          update_play_count(name: player.name, play_count: player.play_count + 1)
        end

        [0,1,2].each do |id|
          player = player_list[id]
          rank_down(name: player.name, udemae: player.udemae)
          update_total_nuri_point(name: player.name, total_nuri_point: player.total_nuri_point + player.point)
          update_play_count(name: player.name, play_count: player.play_count + 1)
        end
      end
    end

    def update_total_nuri_point(name:, total_nuri_point:)
      db.execute("UPDATE ranking_table SET total_nuri_point = '#{total_nuri_point}' where name = '#{name}'")
    end

    def update_play_count(name:, play_count:)
      db.execute("UPDATE ranking_table SET play_count = '#{play_count}' where name = '#{name}'")
    end

    #
    # ランクが上がる
    #
    def rank_up(name:, udemae:)
      /(?<rank>[A-Z\-\+]+)(?<point>[0-9]+)/ =~ udemae
      new_point = (point.to_i + UDEMAE_UP_RATE[rank])

      if new_point > 99
        new_udemae = NEXT_UDEMAE[rank] + '30'
      else
        new_udemae = rank + new_point.to_s
      end

      puts "#{name} rank up! #{udemae} -> #{new_udemae}"
      db.execute("UPDATE ranking_table SET udemae = '#{new_udemae}' where name = '#{name}'")
    end

    #
    # ランクが下る
    #
    def rank_down(name:, udemae:)
      /(?<rank>[A-Z\-\+]+)(?<point>[0-9]+)/ =~ udemae
      new_point = (point.to_i - UDEMAE_DOWN_RATE[rank])

      if new_point < 0
        new_udemae = PRED_UDEMAE[rank] + '70'
      else
        new_udemae = rank + new_point.to_s
      end

      puts "#{name} rank down... #{udemae} -> #{new_udemae}"
      db.execute("UPDATE ranking_table SET udemae = '#{new_udemae}' where name = '#{name}'")
    end

    #
    # データベースの初期化
    #
    def open_db
      @db = SQLite3::Database.new('./db/ranking.db')
    end

    #
    # データベースのクローズ
    #
    def close_db
      db && db.close
    end

    #
    # プレイヤーの登録
    #
    def regist_player(name:)
      db.execute("INSERT INTO ranking_table values('#{name}', 'C30', 0, 0)")
    end

    #
    # 現在は完全ランダムで居館を作成
    # 今後静的に決定する可能性はある
    #
		def init_kyokan_list
      @kyokan_list = []
      check_list = Hash.new(false)

      MAX_PLAYER_NUM.times do |kyokan_id|
        begin
          y = [*0...height].sample
          x = [*0...width].sample
          z = y*width + x
        end unless check_list[z]

        add_kyokan(kyokan_id: kyokan_id, y: y, x: x)
      end
    end

    # 居館を追加
    def add_kyokan(kyokan_id:, y:, x:)
      cell = field[y][x]

      cell.build_kyokan(id: kyokan_id)
      kyokan_list << SamurAI::Kyokan.new(kyokan_id, x, y)
    end

    #
    # プレイヤー情報の初期化を行う。居館の位置に各プレイヤーをセット
    #
    def init_player_list
      @player_list = []
      ignore_list = ['.', '..', '.gitkeep', 'ultra_greedy_man']

      members = (Dir.entries('./players') - ignore_list).sample(6)
      members[0] = 'ultra_greedy_man'

      kyokan_list.each do |kyokan|
        player_id = kyokan.id
        y = kyokan.y
        x = kyokan.x

        create_player(id: player_id, name: members.shift, y: y, x: x)
      end
    end

    #
    # プレイヤーの作成
    #
    def create_player(id:, name:, y:, x:)
      player = SamurAI::Player.new(id: id, name: name, y: y, x: x)
      count = db.get_first_value("select count(*) from ranking_table where name = '#{name}'")

      # まだ登録がされていない場合は登録を行う
      regist_player(name: name) if count.zero?

      db.execute("select * from ranking_table where name = '#{name}'") do |row|
        name, udemae, total_nuri_point, play_count = *row

        player.udemae = udemae
        player.total_nuri_point = total_nuri_point
        player.play_count = play_count
        #puts "name: #{name}, udemae: #{udemae}, total_nuri_point: #{total_nuri_point}, play_count: #{play_count}"
      end

      @player_list[id] = player
    end

    # 試合を始める
    def start_game
      init_kyokan_list
    end

    # 一番最初に渡すパラメータ
    def first_input_params(id:, group_id:)
      params = []

      # 総ターン数 所属 ID 横幅 縦幅
      params << [max_turn, group_id, id, width, height].join(' ')

      # 各サムライの居館の位置を入れる
      kyokan_list.each do |kyokan|
        params << kyokan.position
      end

      # 各サムライの成績を入れる
      player_list.each do |samurai|
        params << samurai.ranking_data
      end

      params.join("\n")
    end

    # 試合中に渡すパラメータ
    def input_params(player_id)
      params = []
      player = player_list[player_id]

      # 現在のターン
      params << current_turn

      # 治療期間
      params << player.cure_period

      # 各プレイヤーの情報
      player_list.each do |samurai|
        params << samurai.info
      end

      # フィールド情報
      params << field.info(player.group_id)

      #STDERR.puts params.join("\n")
      params.join("\n")
    end

    #
    # 行動指示の解析
    # @param input プレイヤーからの行動指示
    #
    def parse_operation(input)
      input.split.map(&:to_i)
    end
	end
end
