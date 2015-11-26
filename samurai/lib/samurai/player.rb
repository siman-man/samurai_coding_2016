module SamurAI
	class Player
    attr_reader :ranking, :status, :y, :x, :id, :group_id

    HIDE = 0

		def initialize(id:, x:, y:, group_id:)
      @y = y
      @x = x
      @id = id
      @group_id = group_id
		end

		def info
    end

    def hide?
      status == HIDE
    end
	end
end
