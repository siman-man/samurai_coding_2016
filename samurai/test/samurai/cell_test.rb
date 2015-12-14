require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiCell < Minitest::Test
  attr_reader :server

  def setup
    @server = SamurAI::Server.new
    @server.init_game
  end

  def test_initialize
    cell = server.field[1][1]

    assert_equal true, cell.neutral?
    assert_equal false, cell.exist_kyokan?
  end

  def test_build_kyokan
    cell = server.field[0][0]
    cell.build_kyokan(id: 1)

    assert_equal 1, cell.owner
  end

  def test_occupy
    cell1 = server.field[1][1]
    cell2 = server.field[2][2]
    cell1.build_kyokan(id: 2)

    cell1.occupy(id: 1)
    assert_equal false, cell1.attacked

    cell2.occupy(id: 1)
    assert_equal true, cell2.attacked
    assert_equal 1, cell2.owner
  end
end
