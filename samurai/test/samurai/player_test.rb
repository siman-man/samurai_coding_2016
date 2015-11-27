require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiPlayer < Minitest::Test
  attr_reader :player

  def setup
    @player = SamurAI::Player.new(id: 0, y: 10, x: 8, group_id: 0)
  end

  def test_initialize
    assert_equal 0, player.id
    assert_equal 10, player.y
    assert_equal 8, player.x
    assert_equal 0, player.group_id
    assert_equal 0, player.cure_period
    assert_equal false, player.hide?
  end

  def test_hide
    player.hide
    assert_equal true, player.hide?
  end

  def test_show_up
    player.show_up
    assert_equal false, player.hide?
  end

  def test_cure
    player.cure_period = 2
    player.cure
    assert_equal 1, player.cure_period

    player.cure
    assert_equal 0, player.cure_period

    player.cure
    assert_equal 0, player.cure_period
  end

  def test_can_visible?
    assert_equal true, player.can_visible?(y: 10, x: 7)
    assert_equal true, player.can_visible?(y: 10, x: 3)
    assert_equal false, player.can_visible?(y: 10, x: 2)
    assert_equal false, player.can_visible?(y: 8, x: 12)
    assert_equal true, player.can_visible?(y: 8, x: 5)
  end
end