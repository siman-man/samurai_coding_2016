module SamurAI
	class Server
		attr_reader :max_turn, :width, :height, :field, :healing_time,
                :kyokan_list, :player_list, :current_turn

    # 参加プレイヤーの最大人数
    MAX_PLAYER_NUM = 6

		NEUTRAL = 8
		UNKNOWN = 9

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

      init_kyokan_list

      @field = Array.new(@height).map{Array.new(@width, NEUTRAL)}
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

        kyokan_list << Kyokan.new(kyokan_id, x, y)
      end
    end

    #
    # プレイヤー情報の初期化を行う
    #
    def init_player_list
      @player_list = []
    end

    # 試合を始める
    def start_game
    end

    # 一番最初に渡すパラメータ
    def first_input_params(group_id:, id:)
      params = []

      # 総ターン数 所属 ID 横幅 縦幅
      params << [max_turn, group_id, id, width, height].join(' ')

      # 各サムライの居館の位置を入れる
      MAX_PLAYER_NUM.times do |kyokan_id|
        kyokan = kyokan_list[kyokan_id]
        params << kyokan.position
      end

      # 各サムライの成績を入れる
      MAX_PLAYER_NUM.times do |player_id|
        player = player_list[player_id]
        params << player.info
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
      MAX_PLAYER_NUM.times do |player_id|
        samurai = player_list[player_id]
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
