module SamurAI
	class Kyokan
		attr_reader :id, :x, :y

		def initialize(id, x, y)
			@id = id
			@x = x
			@y = y
		end

		# 自分の位置を返す
		def position
			[x, y].join(' ')
		end
	end
end
