require_relative '../mini_helper'
require 'minitest/autorun'

class TestSamuraiPlayer < Minitest::Test
  attr_reader :player, :field

  def setup
    @player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    @field = SamurAI::Field.new(width: 15, height: 12)
  end

  def reset_data
    @player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    @field = SamurAI::Field.new(width: 15, height: 12)
  end

  def reset_field
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
    assert_equal true, player.show_up(field: field)
    assert_equal false, player.hide?

    player.hide
    field[10][8].set_samurai(id: 1)
    assert_equal false, player.show_up(field: field)
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
    player.show_up(field: field)
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
    field[10][9].set_samurai(id: 3)

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
    reset_data
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
    field[11][9].update_owner(id: 1)
    assert_equal true, player.move(direct: 1, field: field)
    assert_equal 11, player.y
    assert_equal 9, player.x

    # 北に移動
    field[10][9].update_owner(id: 2)
    assert_equal true, player.move(direct: 2, field: field)
    assert_equal 10, player.y
    assert_equal 9, player.x

    # 敵の陣地でも移動できない
    field[10][8].update_owner(id: 3)
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
    field[10][6].set_samurai(id: 3)
    field[10][6].update_owner(id: 0)

    # 他のサムライがいても行動可能
    assert_equal true, player.move(direct: 3, field: field)
    assert_equal 10, player.y
    assert_equal 6, player.x
  end

  def test_job
    spear_player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    spear_player2 = SamurAI::Player.new(id: 3, y: 10, x: 8)

    assert_equal SamurAI::Player::SPEAR, spear_player.job
    assert_equal SamurAI::Player::SPEAR, spear_player2.job


    sword_player = SamurAI::Player.new(id: 1, y: 10, x: 8)
    sword_player2 = SamurAI::Player.new(id: 4, y: 10, x: 8)

    assert_equal SamurAI::Player::SWORD, sword_player.job
    assert_equal SamurAI::Player::SWORD, sword_player2.job


    ax_player = SamurAI::Player.new(id: 2, y: 10, x: 8)
    ax_player2 = SamurAI::Player.new(id: 5, y: 10, x: 8)

    assert_equal SamurAI::Player::AX, ax_player.job
    assert_equal SamurAI::Player::AX, ax_player2.job
  end

  def test_spear_attack
    reset_field
    spear_player = SamurAI::Player.new(id: 0, y: 10, x: 8)
    spear_player.hide

    # 潜伏中は攻撃出来ない
    assert_equal false, spear_player.attack(direct: 0, field: field)

    spear_player.show_up(field: field)

    # 南方角への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 0, field: field)
    assert_equal true, field[11][8].attacked
    assert_equal spear_player.id, field[11][8].owner
    assert_equal false, field[10][8].attacked
    assert_equal false, field[9][8].attacked

    # 東方角への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 1, field: field)
    assert_equal true, field[10][9].attacked
    assert_equal spear_player.id, field[10][9].owner
    assert_equal true, field[10][10].attacked
    assert_equal spear_player.id, field[10][10].owner
    assert_equal true, field[10][11].attacked
    assert_equal spear_player.id, field[10][11].owner
    assert_equal false, field[10][7].attacked

    # 北方角への攻撃
    assert_equal true, spear_player.attack(direct: 2, field: field)
    assert_equal true, field[9][8].attacked
    assert_equal spear_player.id, field[9][8].owner
    assert_equal true, field[8][8].attacked
    assert_equal spear_player.id, field[8][8].owner
    assert_equal true, field[7][8].attacked
    assert_equal spear_player.id, field[7][8].owner
    assert_equal true, field[6][8].attacked
    assert_equal spear_player.id, field[6][8].owner
    assert_equal false, field[5][8].attacked

    # 西方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 3, field: field)
    assert_equal true, field[10][7].attacked
    assert_equal spear_player.id, field[10][7].owner
    assert_equal true, field[10][6].attacked
    assert_equal spear_player.id, field[10][6].owner
    assert_equal true, field[10][5].attacked
    assert_equal spear_player.id, field[10][5].owner
    assert_equal true, field[10][4].attacked
    assert_equal spear_player.id, field[10][4].owner
    assert_equal false, field[10][3].attacked
    assert_equal false, field[10][9].attacked
  end

  def test_sword_attack
    reset_field
    spear_player = SamurAI::Player.new(id: 1, y: 10, x: 8)

    # 南方向への攻撃
    assert_equal true, spear_player.attack(direct: 0, field: field)
    assert_equal true, field[11][8].attacked
    assert_equal true, field[11][9].attacked
    assert_equal true, field[10][9].attacked
    assert_equal true, field[10][10].attacked
    assert_equal false, field[10][11].attacked
    assert_equal false, field[11][7].attacked
    assert_equal false, field[9][8].attacked
    assert_equal false, field[9][7].attacked

    # 東方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 1, field: field)
    assert_equal true, field[10][9].attacked
    assert_equal true, field[10][10].attacked
    assert_equal false, field[10][11].attacked
    assert_equal true, field[9][9].attacked
    assert_equal true, field[9][8].attacked
    assert_equal true, field[8][8].attacked
    assert_equal false, field[7][8].attacked

    # 北方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 2, field: field)
    assert_equal true, field[9][8].attacked
    assert_equal true, field[8][8].attacked
    assert_equal true, field[9][7].attacked
    assert_equal true, field[10][7].attacked
    assert_equal true, field[10][6].attacked
    assert_equal false, field[7][8].attacked
    assert_equal false, field[9][9].attacked
    assert_equal false, field[10][9].attacked
    assert_equal false, field[10][5].attacked
    assert_equal false, field[10][10].attacked
    assert_equal false, field[10][11].attacked

    # 西方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 3, field: field)
    assert_equal true, field[10][7].attacked
    assert_equal true, field[10][6].attacked
    assert_equal true, field[11][7].attacked
    assert_equal true, field[11][8].attacked
    assert_equal false, field[10][5].attacked
    assert_equal false, field[10][8].attacked
    assert_equal false, field[11][6].attacked
  end

  def test_ax_attack
    reset_field
    spear_player = SamurAI::Player.new(id: 2, y: 10, x: 8)

    # 南方向への攻撃
    assert_equal true, spear_player.attack(direct: 0, field: field)
    assert_equal true, field[11][8].attacked
    assert_equal true, field[11][9].attacked
    assert_equal true, field[11][7].attacked
    assert_equal true, field[10][9].attacked
    assert_equal true, field[10][7].attacked
    assert_equal true, field[9][9].attacked
    assert_equal true, field[9][7].attacked
    assert_equal false, field[9][8].attacked
    assert_equal false, field[9][6].attacked
    assert_equal false, field[10][8].attacked

    # 東方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 1, field: field)
    assert_equal true, field[9][9].attacked
    assert_equal true, field[10][9].attacked
    assert_equal true, field[11][9].attacked
    assert_equal true, field[9][8].attacked
    assert_equal true, field[11][8].attacked
    assert_equal true, field[9][7].attacked
    assert_equal true, field[11][7].attacked
    assert_equal false, field[10][7].attacked

    # 北方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 2, field: field)
    assert_equal true, field[9][7].attacked
    assert_equal true, field[9][8].attacked
    assert_equal true, field[9][9].attacked
    assert_equal true, field[10][7].attacked
    assert_equal true, field[10][9].attacked
    assert_equal true, field[11][7].attacked
    assert_equal true, field[11][9].attacked
    assert_equal false, field[11][8].attacked

    # 西方向への攻撃
    reset_field
    assert_equal true, spear_player.attack(direct: 3, field: field)
    assert_equal true, field[9][7].attacked
    assert_equal true, field[10][7].attacked
    assert_equal true, field[11][7].attacked
    assert_equal true, field[9][8].attacked
    assert_equal true, field[11][8].attacked
    assert_equal true, field[9][9].attacked
    assert_equal true, field[11][9].attacked
    assert_equal false, field[10][9].attacked
    assert_equal false, field[10][8].attacked
  end

  def test_attack_not_occupy_kyokan
    reset_field
    ax_player = SamurAI::Player.new(id: 2, y: 10, x: 8)

    field[11][8].build_kyokan(id: 1)
    ax_player.attack(direct: 0, field: field)

    assert_equal 1, field[11][8].owner
  end
end