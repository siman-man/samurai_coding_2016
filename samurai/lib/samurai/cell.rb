module SamurAI
  class Cell
    attr_reader :status, :owner, :attacked, :samurai_id
    attr_accessor :visible

    NEUTRAL = 8
    UNKNOWN = 9

    def initialize
      @owner = NEUTRAL
      @exist_kyokan = false
      @samurai_id = nil
      @attacked = false
      @visible = Array.new(2, false)
    end

    #
    # 居館が存在しているかどうかを調べる
    #
    def exist_kyokan?
      @exist_kyokan
    end

    #
    # 潜伏していないサムライが存在しているかどうかを調べる
    #
    def exist_samurai?
      @samurai_id
    end

    #
    # サムライがいることを更新
    #
    def set_samurai(id:)
      @samurai_id = id
    end

    #
    # 居館を建築
    #
    def build_kyokan(id:)
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
    def update_owner(id:)
      @exist_kyokan || @owner = id
    end

    #
    # このセルを所有しているチームを返す、どちらでもない場合は-1を返す
    #
    def owner_group
      if owner == NEUTRAL
        -1
      else
        owner / 3
      end
    end

    #
    # 指定したIDで占領する
    #
    def occupy(id:)
      @attacked = true
      update_owner(id: id)
    end

    #
    # セル情報の初期化を行う
    #   1. セルが攻撃されたかどうかを初期化
    #   2. セルに存在しているサムライを初期化
    #   3. 視界情報を初期化
    #
    def clear
      @attacked = false
      @samurai_id = nil
      @visible = Array.new(2, false)
    end
  end
end