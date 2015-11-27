module SamurAI
  class Player
    attr_reader :ranking, :status, :y, :x, :id, :group_id
    attr_accessor :cure_period

    HIDE = 0
    NOHIDE = 1

    def initialize(id:, x:, y:, group_id:)
      @y = y
      @x = x
      @id = id
      @status = NOHIDE
      @group_id = group_id
      @cure_period = 0
    end

    #
    # 現在の自分の情報を渡す
    #
    def info
      [y, x, status].join(' ')
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
  end
end
