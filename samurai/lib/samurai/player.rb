module SamurAI
  class Player
    attr_reader :ranking, :status, :y, :x, :id, :group_id, :name
    attr_accessor :cure_period, :udemae, :total_nuri_point, :play_count, :point

    #
    # 槍の攻撃範囲
    #
    SPEAR_ATTACK_RANGE = [
      [0, 0, 0, 0, 4, 0, 0, 0, 0],
      [0, 0, 0, 0, 4, 0, 0, 0, 0],
      [0, 0, 0, 0, 4, 0, 0, 0, 0],
      [0, 0, 0, 0, 4, 0, 0, 0, 0],
      [8, 8, 8, 8, 0, 2, 2, 2, 2],
      [0, 0, 0, 0, 1, 0, 0, 0, 0],
      [0, 0, 0, 0, 1, 0, 0, 0, 0],
      [0, 0, 0, 0, 1, 0, 0, 0, 0],
      [0, 0, 0, 0, 1, 0, 0, 0, 0]
    ].freeze

    #
    # 剣の攻撃範囲
    #
    SWORD_ATTACK_RANGE = [
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 6, 0, 0, 0, 0],
      [0, 0, 0, 4, 6, 2, 0, 0, 0],
      [0, 0,12,12, 0, 3, 3, 0, 0],
      [0, 0, 0, 8, 9, 1, 0, 0, 0],
      [0, 0, 0, 0, 9, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0]
    ].freeze

    #
    # マサカリの攻撃範囲
    #
    AX_ATTACK_RANGE = [
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0,15,14,15, 0, 0, 0],
      [0, 0, 0,13, 0, 7, 0, 0, 0],
      [0, 0, 0,15,11,15, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0, 0],
    ].freeze

    #
    # 攻撃範囲をまとめたリスト
    #
    ATTACK_RANGE_LIST = [SPEAR_ATTACK_RANGE, SWORD_ATTACK_RANGE, AX_ATTACK_RANGE]

    #
    # プレイヤーの状態を示す
    # 0: 潜伏状態
    # 1: 潜伏していない状態
    #
    HIDE = 0
    NOHIDE = 1

    #
    # 視界の範囲
    #
    SIGHT_RANGE = 5

    #
    # 職業タイプ
    #   0. 槍
    #   1. 剣
    #   2. マサカリ
    #
    SPEAR = 0
    SWORD = 1
    AX = 2

    #
    # 方角は「南、東、北、西」の順番
    #
    DY = [1, 0, -1, 0].freeze
    DX = [0, 1, 0, -1].freeze

    def initialize(id:, name:, x:, y:)
      @y = y
      @x = x
      @home_y = y
      @home_x = x
      @id = id
      @name = name
      @point = 0
			@ranking = id
      @status = NOHIDE
      @group_id = id / 3 # [0,1,2], [3,4,5] でグループ分け
      @cure_period = 0
    end

    #
    # プレイヤーをロードする
    #
    def load
      @player = IO.popen("players/#{name}", 'r+')
    end

    #
    # プレイヤーに対して情報を送る
    #
    def input(params)
      @player.puts(params)
    end

    #
    # プレイヤーからの情報を受取る
    #
    def response
      @player.gets
    end

    #
    # 現在の自分の情報を渡す
    #
    def info
      [y, x, status].join(' ')
    end

    #
    #
    #
    def detail
      "name: #{name}, udemae: #{udemae}, total_nuri_point: #{total_nuri_point}, play_count: #{play_count}"
    end

    #
    # 居館に戻る
    #
    def go_back_home
      @y = @home_y
      @x = @home_x
    end

    #
    # プレイヤーの戦績を返す
    #
    def ranking_data
      [ranking, point].join(' ')
    end

    #
    # 移動を行う、その場所に移動できない場合はfalseを返す
    #   1. 潜伏状態で味方陣地以外のところに移動する
    #   2. 居館がある区間には移動できない
    #
    def move(direct:, field:)
      ny = y + DY[direct]
      nx = x + DX[direct]

      # フィールド外に出ていた場合は失敗
      return false if field.outside?(y: ny, x: nx)

      cell = field[ny][nx]

      # 他のサムライの居館が存在している場合も失敗
      return false if cell.exist_kyokan? && cell.owner != id

      if hide?
        # 潜伏状態は味方領地しか移動できない
        if cell.owner_group == group_id
          @y = ny
          @x = nx
        else
          return false
        end
      else
        # 潜伏していない状態で他のサムライが存在している場合は失敗
        if cell.exist_samurai?
          return false
        else
          @y = ny
          @x = nx
        end
      end

      true
    end

    #
    # 攻撃（占領）
    #   1. 画面外は無視
    #   2. 攻撃範囲のフィールドにチェックをつける
    #   3. 潜伏状態では攻撃出来ない
    #
    def attack(direct:, field:)
      return false if hide?

      ATTACK_RANGE_LIST[job].each.with_index(y-4) do |row, ny|
        row.each.with_index(x-4) do |mask, nx|
          if field.inside?(y: ny, x: nx) && ((mask >> direct) & 1) == 1
            field[ny][nx].occupy(id: id)
          end
        end
      end

      true
    end

    #
    # 潜伏する。味方の陣地以外の場所では出来ない
    #
    def hide(field:)
      if field[y][x].owner_group == group_id
        @status = HIDE
      else
        return false
      end

      true
    end

    #
    # 姿を見せる。出現先に他のサムライがいた場合は失敗する
    #
    def show_up(field:)
      if field[y][x].exist_samurai?
        return false
      else
        @status = NOHIDE
      end

      true
    end

    #
    # 現在潜伏中かどうか
    #
    def hide?
      status == HIDE
    end

    #
    # 回復
    #
    def cure
      @cure_period = [0, @cure_period-1].max
    end

    #
    # 視界情報の更新
    #
    def view(field:)
      (0...field.height).each do |y|
        (0...field.width).each do |x|
          if can_visible?(y: y, x: x)
            field[y][x].visible[group_id] = true
          end
        end
      end
    end

    #
    # 指定されたマスが見えるかどうか
    #
    def can_visible?(y:, x:)
      (@x - x).abs + (@y - y).abs <= SIGHT_RANGE
    end

    #
    # プレイヤーの職業
    #
    def job
      id % 3
    end
  end
end
