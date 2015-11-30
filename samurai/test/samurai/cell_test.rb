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
end
