require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiField < Minitest::Test
  attr_reader :field

  def setup
    @field = SamurAI::Field.new(width: 15, height: 10)
  end

  def test_initialize
    assert_equal 10, field.height
    assert_equal 15, field.width
  end

  def test_inside?
    assert_equal true, field.inside?(y: 0, x: 0)
    assert_equal true, field.inside?(y: 5, x: 10)
    assert_equal false, field.inside?(y: 10, x: 10)
    assert_equal false, field.inside?(y: 10, x: -1)
  end
end