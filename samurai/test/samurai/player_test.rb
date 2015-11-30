require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiPlayer < Minitest::Test
  attr_reader :player, :field

  def setup
    @player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    @field = SamurAI::Field.new(width: 15, height: 12)
  end

  def reset
    @player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    @field = SamurAI::Field.new(width: 15, height: 12)
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

  #
  # 姿が見えている時の移動行動
  #
  def test_move_show_up
    player.show_up
    # 南に移動
    assert_equal true, player.move(direct: 0, field: field)
    assert_equal 11, player.y
    assert_equal 8, player.x

    # 画面外への移動は失敗する（プレイヤーの座標はそのまま）
    assert_equal false, player.move(direct: 0, field: field)
    assert_equal 11, player.y
    assert_equal 8, player.x

    # 東に移動
    assert_equal true, player.move(direct: 1, field: field)
    assert_equal 11, player.y
    assert_equal 9, player.x

    # 北に移動
    assert_equal true, player.move(direct: 2, field: field)
    assert_equal 10, player.y
    assert_equal 9, player.x

    # 西に移動
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    # (10, 9)の位置にサムライをセット
    field[10][9].set_samurai

    # 既にサムライが居る場所には移動できない（プレイヤーの座標はそのまま）
    assert_equal false, player.move(direct: 1, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    # (11, 8)の位置に他のサムライの居館を立てる
    field[11][8].build_kyokan(id: 1)

    # (10, 7)の位置に自分のサムライの居館を立てる
    field[10][7].build_kyokan(id: 0)

    # 他のサムライの居館がある場所には移動できない
    assert_equal false, player.move(direct: 0, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    # 自分の居館であれば移動可
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 7, player.x
  end

  #
  # 姿を隠している時の行動
  #
  def test_move_hide
    # フィールドとプレイヤーを初期化
    reset
    player.hide

    # 隠れている状態では味方の陣地以外は行動出来ない
    assert_equal false, player.move(direct: 0, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    field[11][8].update_owner(id: 0)

    # 味方の陣地であれば行動出来る
    assert_equal true, player.move(direct: 0, field: field)
    assert_equal 11, player.y
    assert_equal 8, player.x

    # 画面外への移動は失敗する（プレイヤーの座標はそのまま）
    assert_equal false, player.move(direct: 0, field: field)
    assert_equal 11, player.y
    assert_equal 8, player.x

    # 東に移動
    field[11][9].update_owner(id: 2)
    assert_equal true, player.move(direct: 1, field: field)
    assert_equal 11, player.y
    assert_equal 9, player.x

    # 北に移動
    field[10][9].update_owner(id: 4)
    assert_equal true, player.move(direct: 2, field: field)
    assert_equal 10, player.y
    assert_equal 9, player.x

    # 敵の陣地でも移動できない
    field[10][8].update_owner(id: 1)
    assert_equal false, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 9, player.x

    field[10][8].update_owner(id: 0)
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    # (11, 8)の位置に他のサムライの居館を立てる
    field[11][8].build_kyokan(id: 1)

    # 他のサムライの居館の位置には移動できない
    assert_equal false, player.move(direct: 0, field: field)
    assert_equal 10, player.y
    assert_equal 8, player.x

    # (10, 7)の位置に自分のサムライの居館を立てる
    field[10][7].build_kyokan(id: 0)

    # 自分の居館であれば移動可
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 7, player.x

    # (10, 6)の位置にサムライをセット
    field[10][6].set_samurai
    field[10][6].update_owner(id: 0)

    # 他のサムライがいても行動可能
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 6, player.x
  end
end