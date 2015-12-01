module SamurAI
	class Server
		attr_reader :max_turn, :width, :height, :field, :healing_time,
                :kyokan_list, :player_list, :current_turn

    # 参加プレイヤーの最大人数
    MAX_PLAYER_NUM = 6

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
      @max_turn = 12 * [*1..84].sample
      @width = [*10..20].sample
      @height = [*10..20].sample
      @healing_time = [*12..48].sample
      @current_turn = 0

      create_field(width: width, height: height)
      init_kyokan_list
      init_player_list
    end

    def create_field(width:, height:)
      @field = SamurAI::Field.new(width: width, height: height)
    end

    #
    # 渡された命令を処理する。
    # コストをオーバーすると失敗
    # @params operation_list 命令リスト
    #
    def exec_operation(player:, operation_list:)
      cost = 0

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

      kyokan_list.each do |kyokan|
        player_id = kyokan.id
        y = kyokan.y
        x = kyokan.x

        create_player(id: player_id, y: y, x: x)
      end
    end

    #
    # プレイヤーの作成
    #
    def create_player(id:, y:, x:)
      @player_list[id] = SamurAI::Player.new(id: id, y: y, x: x)
    end

    # 試合を始める
    def start_game
      init_kyokan_list
    end

    # 一番最初に渡すパラメータ
    def first_input_params(group_id:, id:)
      params = []

      # 総ターン数 所属 ID 横幅 縦幅
      params << [max_turn, group_id, id, width, height].join(' ')

      # 各サムライの居館の位置を入れる
      kyokan_list.each do |kyokan|
        params << kyokan.position
      end

      # 各サムライの成績を入れる
      player_list.each do |samurai|
        params << samurai.data
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
