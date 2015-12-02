require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiServer < Minitest::Test
	attr_reader :server

	def setup
		@server = SamurAI::Server.new
    @server.init_game
  end

  def reset_field
    @server.create_field(width: 10, height: 12)
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
    server.init_kyokan_list

    assert_equal 6, server.kyokan_list.size

    server.kyokan_list.each do |kyokan|
      assert_includes 0..5, kyokan.id
      assert_includes 0...server.width, kyokan.x
      assert_includes 0...server.height, kyokan.y
    end
  end

  def test_init_player_list
    assert_equal 6, server.player_list.size

    server.player_list.each do |player|
      assert_includes 0..5, player.id
      assert_includes 0...server.width, player.x
      assert_includes 0...server.height, player.y
    end
  end

  def test_parse_operation
    operation = "0 0 0 0 0 0 0"
    assert_equal [0, 0, 0, 0, 0, 0, 0], server.parse_operation(operation)

    operation = "1 2 3 4 5 6 7"
    assert_equal [1, 2, 3, 4, 5, 6, 7], server.parse_operation(operation)
  end

  #
  # 命令リストの実行テスト その1
  #
  def test_exec_operation_01
    reset_field
    spear_player = SamurAI::Player.new(id: 0, name: 'siman', y: 5, x: 5)
    server.field[6][5].occupy(id: 0)

    #
    # 1. 南方向に移動
    # 2. 南方向に攻撃
    # 3. 潜伏
    #
    operation_list = [5, 1, 9, 0, 0, 0, 0]
    server.exec_operation(player: spear_player, operation_list: operation_list)

    # 南方向に行動出来ている
    assert_equal 6, spear_player.y
    assert_equal 5, spear_player.x

    # 南方向に攻撃出来ている
    assert_equal true, server.field[7][5].attacked
    assert_equal true, server.field[8][5].attacked
    assert_equal true, server.field[9][5].attacked
    assert_equal true, server.field[10][5].attacked
    assert_equal false, server.field[11][5].attacked

    # 潜伏している
    assert_equal true, spear_player.hide?
  end

  #
  # 命令リストの実行テスト その2
  #
  def test_exec_operation_02
    reset_field
    spear_player = SamurAI::Player.new(id: 0, name: 'siman', y: 5, x: 5)
    server.field[5][5].occupy(id: 0)

    #
    # 潜伏と顕現を繰り返す
    #
    operation_list = [9, 10, 9, 10, 9, 10, 9]
    server.exec_operation(player: spear_player, operation_list: operation_list)

    assert_equal true, spear_player.hide?
  end
end
