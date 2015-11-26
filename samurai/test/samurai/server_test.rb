require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamurai < Minitest::Test
	attr_reader :server

	def setup
		@server = SamurAI::Server.new
    @server.init_game
  end

  # 初期化の時は0になっている
  def test_current_turn
    assert_equal 0, server.current_turn
  end

	# 総ターン数は 12 の倍数であり、最小値は 12, 最大値は 1008 である。
	def test_max_turn
		assert_equal 0, server.max_turn%12
		assert_includes 12..1008, server.max_turn
	end

	# 戦場の横幅の最小値は 10, 最大値は 20 である。
	def test_width
		assert_includes 10..20, server.width
	end

	# 戦場の縦幅の最小値は 10, 最大値は 20 である。
	def test_height
		assert_includes 10..20, server.height
	end

	# 治療期間の最小値は 12, 最大値は 48である。
	def test_healing_time
		assert_includes 12..48, server.healing_time
	end

	def test_init_kyokan_list
    6.times do |id|
      kyokan = server.kyokan_list[id]

      assert_includes 0...6, kyokan.id
      assert_includes 0...server.width, kyokan.x
      assert_includes 0...server.height, kyokan.y
    end
  end
end
