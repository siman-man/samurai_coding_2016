module SamurAI
  class Cell
    attr_reader :status, :owner

    NEUTRAL = 8
    UNKNOWN = 9

    def initialize
      @owner = NEUTRAL
      @exist_kyokan = false
      @visible = Array.new(2, false)
    end

    #
    # 居館が存在しているかどうか
    #
    def exist_kyokan?
      @exist_kyokan
    end

    #
    # 居館を建築
    #
    def build_kyokan(id)
      @owner = id
      @exist_kyokan = true
    end

    #
    # 現在NEUTRALかどうかを調べる
    #
    def neutral?
      @owner == NEUTRAL
    end

    #
    # 指定したグループがこのセルを見れているかどうか
    #
    def visible?(group_id)
      @visible[group_id]
    end

    #
    # 所有者の更新(居館が存在している場合は更新しない)
    #
    def update_owner(id)
      @exist_kyokan || @owner = id
    end

    #
    # このセルを所有しているチームを返す、どちらでもない場合は-1を返す
    #
    def owner_group
      if owner == NEUTRAL
        -1
      else
        owner%2
      end
    end
  end
end