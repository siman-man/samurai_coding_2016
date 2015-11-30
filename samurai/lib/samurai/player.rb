module SamurAI
  class Player
    attr_reader :ranking, :point, :status, :y, :x, :id, :group_id
    attr_accessor :cure_period

    # プレイヤーの状態を示す
    HIDE = 0
    NOHIDE = 1

    SIGHT_RANGE = 5 # 視界の範囲

    # 方角は「南、東、北、西」の順番
    DY = [1, 0, -1, 0]
    DX = [0, 1, 0, -1]

    def initialize(id:, x:, y:)
      @y = y
      @x = x
      @id = id
      @point = 0
      @status = NOHIDE
      @group_id = id % 2
      @cure_period = 0
    end

    #
    # 現在の自分の情報を渡す
    #
    def info
      [y, x, status].join(' ')
    end

    #
    # プレイヤーの戦績を返す
    #
    def data
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
    # 潜伏
    #
    def hide
      @status = HIDE
    end

    #
    # 姿を見せる
    #
    def show_up
      @status = NOHIDE
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
    # 指定されたマスが見えるかどうか
    #
    def can_visible?(y:, x:)
      (@x - x).abs + (@y - y).abs <= SIGHT_RANGE
    end
  end
end
