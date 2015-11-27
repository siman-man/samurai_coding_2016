require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiVersion < Minitest::Test
	def test_version
		assert_equal '0.0.1', SamurAI::VERSION
	end
end
