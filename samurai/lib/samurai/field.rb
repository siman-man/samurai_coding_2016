module SamurAI
  class Field
    attr_reader :width, :height, :visible

    NEUTRAL = 8
    UNKNOWN = 9

    def initialize(width:, height:)
      @width = width
      @height = height
      @field = Array.new(height).map { Array.new(width).map{Cell.new} }
      @visible = Array.new(height).map { Array.new(width, NEUTRAL).map { Array.new(2, false) } }
    end

    #
    # フィールド情報の更新
    #
    def update(player_list)
      # 視界情報を初期化
      @visible = Array.new(height).map { Array.new(width, NEUTRAL).map { Array.new(2, false) } }
    end

    #
    # 指定したgroup_idから確認できるフィールド情報を返す
    #
    def info(group_id)
      @field.map.with_index { |row, y|
        row.map.with_index { |cell, x|
          if cell.visible?(group_id)
            cell.status
          else
            UNKNOWN
          end
        }.join(' ')
      }.join("\n")
    end

    def [](index)
      @field[index]
    end

    def inside?(y:, x:)
      0 <= y && y < height && 0 <= x && x < width
    end

    def outside?(y:, x:)
      y < 0 || height <= y || x < 0 || width <= x
    end
  end
end