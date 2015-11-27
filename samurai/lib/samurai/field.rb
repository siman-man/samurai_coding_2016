module SamurAI
  class Field
    attr_reader :width, :height, :visible

    NEUTRAL = 8
    UNKNOWN = 9

    def initialize(width:, height:)
      @width = width
      @height = height
      @field = Array.new(height).map{Array.new(width, NEUTRAL)}
      @visible = Array.new(height).map{Array.new(width, NEUTRAL).map{Array.new(2, false)}}
    end

    #
    # フィールド情報の更新
    #
    def update
      # 視界情報を初期化
      @visible = Array.new(height).map{Array.new(width, NEUTRAL).map{Array.new(2, false)}}
    end

    #
    # 指定したgroup_idから確認できるフィールド情報を返す
    #
    def info(group_id)
      field.map.with_index do |row, y|
        row.map.with_index do |cell, x|
          if visible[y][x][group_id]
            cell
          else
            UNKNOWN
          end
        end
      end
    end

    def [](index)
      field[index]
    end
  end
end