module SamurAI
	class Server
		attr_reader :max_turn, :width, :height, :field, :healing_time, :kyokan_list

		NEUTRAL = 8
		UNKNOWN = 9

		def initialize
			@max_turn = 12 * [*1..84].sample
			@width = [*10..20].sample
			@height = [*10..20].sample
			@healing_time = [*12..48].sample

			init_kyokan

			@field = Array.new(@height).map{Array.new(@width, NEUTRAL)}
		end

		def init_kyokan
			@kyokan_list = Array.new(12)
		end
	end
end
