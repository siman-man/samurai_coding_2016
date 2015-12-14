#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <algorithm>
#include <limits.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;

const int NOHIDE = 0;
const int HIDE = 1;
const int UNKNOWN = -1;
const int NEUTRAL = 8;
const int NOTHING = 9;
const int SPEAR = 0;
const int SWORD = 1;
const int AX = 2;

const int DY[4] = {1, 0, -1, 0};
const int DX[4] = {0, 1, 0, -1};

const int MAX_COST = 7;

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

struct COORD {
  int y;
  int x;
  int dist;

  COORD(int y = UNKNOWN, int x = UNKNOWN, int dist = 0){
    this->y = y;
    this->x = x;
    this->dist = dist;
  }
};

struct KYOKAN{
  int id;       // 居館番号
  int y;        // y座標
  int x;        // x座標

  KYOKAN(int id = UNKNOWN, int y = UNKNOWN, int x = UNKNOWN){
    this->id = id;
    this->y = y;
    this->x = x;
  }
};

struct EVAL {
  int owner_count; // 自身の領土の数

  EVAL(){
    this->owner_count = 0;
  }

  bool operator >(const EVAL &e) const{
    return owner_count < e.owner_count;
  }    
};

struct NODE {
  vector<int> operation_list;
  double eval;

  bool operator >(const NODE &e) const{
    return eval < e.eval;
  }    
};

struct PLAYER{
  int id;				    // サムライ番号
  int group_id;     // グループID
  int y;				    // 現在のy座標
  int x;				    // 現在のx座標
  int beforeY;      // 前のターンのy座標
  int beforeX;      // 前のターンのx座標
  int job;          // 職種
  int homeY;		    // 居館のy座標
  int homeX; 		    // 居館のx座標
  int status; 	    // 潜伏状態かどうか
  int beforeStatus; // 前回の状態
  int rank;         // 現在のランキング
  int score;        // 現在のスコア
  int update_at;    // 更新ターン

  PLAYER(int id = UNKNOWN){
    this->id = id;
    this->y = UNKNOWN;
    this->x = UNKNOWN;
    this->y = beforeY;
    this->x = beforeX;
    this->job = id%3;
    this->homeY = UNKNOWN;
    this->homeX = UNKNOWN;
    this->beforeStatus = 0;
  }

  // 初期位置の設定
  void setHomePosition(int y, int x){
    this->y = y;
    this->x = x;
    this->homeY = y;
    this->homeX = x;
  }

  // プレイヤー情報の更新
  void update(int y, int x, int status){
    this->beforeY = this->y;
    this->beforeX = this->x;
    this->beforeStatus = this->status;
    this->y = y;
    this->x = x;
    this->status = status;
  }
};

// 参加するプレイヤーの最大人数
const int MAX_PLAYER_NUM = 6;

// フィールドの最大横幅
const int MAX_WIDTH = 20;
// フィールドの最大縦幅
const int MAX_HEIGHT = 20;

// プレイヤーのリスト
PLAYER g_playerList[MAX_PLAYER_NUM];

// 居館のリスト
KYOKAN g_kyokanList[MAX_PLAYER_NUM];

// 自分のプレイヤー番号
int g_playerId;
// グループID
int g_group_id;
// 最大ターン数
int g_max_turn;
// フィールドの横幅
int g_width;
// フィールドの縦幅
int g_height;
// 現在のターン
int g_current_turn;
// 治療期間
int g_cure_period;
// フィールド
vector< vector<int> > g_field;
// 一時保存用
vector< vector<int> > g_temp_field;
// 前のターンのフィールド情報
vector< vector<int> > g_before_field;

// そのフィールドの危険度を表す
vector< vector<int> > g_danger_field;
vector< vector<int> > g_temp_danger_field;

// 敵から見られているかどうかを表す
vector< vector<bool> > g_visibled_field;

// 倒した数
int g_kill_count = 0;

const int ACTION_PATTERN_NUM = 41;
const int OPERATION_LENGTH = 7;

/*
 * プレイヤーの行動パターン
 */
int action_pattern[ACTION_PATTERN_NUM][7] = {
  // 何もしない
  {0, 0, 0, 0, 0, 0, 0},

  // 2回行動(行って戻る動作は意味ないので排除)
  {5, 5, 0, 0, 0, 0, 0}, // (-2, 0)
  {5, 6, 0, 0, 0, 0, 0}, // (-1, 1)
  {5, 8, 0, 0, 0, 0, 0}, // (-1,-1)
  {6, 6, 0, 0, 0, 0, 0}, // ( 0, 2)
  {6, 7, 0, 0, 0, 0, 0}, // ( 1, 1)
  {7, 7, 0, 0, 0, 0, 0}, // ( 2, 0)
  {7, 8, 0, 0, 0, 0, 0}, // ( 1,-1)
  {8, 8, 0, 0, 0, 0, 0}, // ( 0,-2)

  // 1回行動 + 攻撃
  {5, 1, 0, 0, 0, 0, 0},
  {5, 2, 0, 0, 0, 0, 0},
  {5, 3, 0, 0, 0, 0, 0},
  {5, 4, 0, 0, 0, 0, 0},
  {6, 1, 0, 0, 0, 0, 0},
  {6, 2, 0, 0, 0, 0, 0},
  {6, 3, 0, 0, 0, 0, 0},
  {6, 4, 0, 0, 0, 0, 0},
  {7, 1, 0, 0, 0, 0, 0},
  {7, 2, 0, 0, 0, 0, 0},
  {7, 3, 0, 0, 0, 0, 0},
  {7, 4, 0, 0, 0, 0, 0},
  {8, 1, 0, 0, 0, 0, 0},
  {8, 2, 0, 0, 0, 0, 0},
  {8, 3, 0, 0, 0, 0, 0},
  {8, 4, 0, 0, 0, 0, 0},

  // 攻撃 + 1回行動
  {1, 5, 0, 0, 0, 0, 0},
  {1, 6, 0, 0, 0, 0, 0},
  {1, 7, 0, 0, 0, 0, 0},
  {1, 8, 0, 0, 0, 0, 0},
  {2, 5, 0, 0, 0, 0, 0},
  {2, 6, 0, 0, 0, 0, 0},
  {2, 7, 0, 0, 0, 0, 0},
  {2, 8, 0, 0, 0, 0, 0},
  {3, 5, 0, 0, 0, 0, 0},
  {3, 6, 0, 0, 0, 0, 0},
  {3, 7, 0, 0, 0, 0, 0},
  {3, 8, 0, 0, 0, 0, 0},
  {4, 5, 0, 0, 0, 0, 0},
  {4, 6, 0, 0, 0, 0, 0},
  {4, 7, 0, 0, 0, 0, 0},
  {4, 8, 0, 0, 0, 0, 0},
};

// プレイヤーの攻撃範囲
int PLAYER_ATTACK_RANGE[3][9][9] = {
  // 槍の攻撃範囲
  {
    {0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 4, 0, 0, 0, 0},
    {8, 8, 8, 8, 0, 2, 2, 2, 2},
    {0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0}
  },

  // 剣の攻撃範囲
  {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 6, 0, 0, 0, 0},
    {0, 0, 0, 4, 6, 2, 0, 0, 0},
    {0, 0,12,12, 0, 3, 3, 0, 0},
    {0, 0, 0, 8, 9, 1, 0, 0, 0},
    {0, 0, 0, 0, 9, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}
  },

  // マサカリの攻撃範囲
  {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0,15,14,15, 0, 0, 0},
    {0, 0, 0,13, 0, 7, 0, 0, 0},
    {0, 0, 0,15,11,15, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}
  }
};

/*
 * 各武器の攻撃範囲
 */
int PLAYER_KILL_RANGE[3][11][11] = {
  // 槍が倒せる範囲
  {
    {0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,2,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0},
  },

  // 剣が倒せる範囲
  {
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,2,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
  },

  // マサカリが倒せる範囲
  {
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,2,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0},
  }
};

class SamurAI{
  public:
    /**
     * 初期情報の読み込みを行う
     */
    void init(){
      scanf("%d %d %d %d %d", &g_max_turn, &g_group_id, &g_playerId, &g_width, &g_height);
      /*
      fprintf(stderr,"ready =>\n");

      fprintf(stderr,"max_turn => %d\n", g_max_turn);
      fprintf(stderr,"player id => %d\n", g_playerId);
      fprintf(stderr,"group id => %d\n", g_group_id);
      fprintf(stderr,"width: %d, height: %d\n", g_width, g_height);
      */
      g_field = vector< vector<int> >(g_height, vector<int>(g_width, 0));
      g_temp_field = vector< vector<int> >(g_height, vector<int>(g_width, 0));

      // 居館の位置を取得（ついでにユーザの初期位置を設定）
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        int homeY;
        int homeX;

        KYOKAN *kyokan = get_kyokan(player_id);
        PLAYER *player = get_player(player_id);
        scanf("%d %d", &homeX, &homeY);

        kyokan->id = player_id;
        kyokan->y = homeY;
        kyokan->x = homeX;

        //fprintf(stderr,"id = %d, y = %d, x = %d\n", player_id, homeY, homeX);

        player->id = player_id;
        player->group_id = player->id/3;
        // プレイヤーの初期位置を初期化
        player->setHomePosition(homeY, homeX);
      }

      // 各プレイヤーの戦績を取得
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int rank;
        int score;

        PLAYER *player = get_player(id);
        scanf("%d %d", &rank, &score);

        player->job = player->id%3;
        player->beforeY = player->y;
        player->beforeX = player->x;
        player->rank = rank;
        player->score = score;

        //fprintf(stderr,"id: %d, rank: %d, score: %d, job = %d\n", id, rank, score, player->job);
      }

      // 準備完了の通知をサーバ側に足して行う
      cout << 0 << endl;
    }

    /**
     * フィールド情報の更新を行う
     */
    bool update_game_data(){
      // 現在のターンの取得
      int status = scanf("%d", &g_current_turn);

      g_danger_field = vector< vector<int> >(g_height, vector<int>(g_width, 0));
      g_visibled_field = vector< vector<bool> >(g_height, vector<bool>(g_width, false));

      if(status == -1){
        return false;
      }

      //fprintf(stderr,"currentTurn => %d\n", g_current_turn);

      // 治療期間の取得
      scanf("%d", &g_cure_period);

      if(g_cure_period > 0){
        //fprintf(stderr,"%d: I'm sleepy now...\n", g_cure_period);
      }

      // 各プレイヤーの情報を更新
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int y;
        int x;
        int status;

        PLAYER *player = get_player(id);
        scanf("%d %d %d", &x, &y, &status);

        // プレイヤーの情報が取得出来た場合
        if(status != UNKNOWN){
          player->update(y, x, status);
          player->update_at = g_current_turn;
        }

        //fprintf(stderr,"id: %d, y: %d, x: %d, status: %d\n", id, player->y, player->x, player->status);
        //fprintf(stderr,"input id: %d, y: %d, x: %d, status: %d\n", id, y, x, status);

        //fprintf(stderr,"id: %d, y: %d, x: %d, beforeY = %d, beforeX = %d, status: %d\n", id, player->y, player->x, player->beforeY, player->beforeX, player->status);
      }

      // 前のフィールド情報を保存する
      g_before_field = g_field;

      // フィールド情報の更新
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int cell;
          scanf("%d", &cell);
          g_field[y][x] = cell;
        }
      }

      update_field_value();
      // 敵の視野をセットする
      set_enemy_sight();

      return true;
    }

    /**
     * 行動
     */
    vector<int> action(){
      vector<int> operation_list(OPERATION_LENGTH, 0);

      priority_queue< NODE, vector<NODE>, greater<NODE> > pque;

      // 盤面を保存
      save_field();

      for(int i = 0; i < ACTION_PATTERN_NUM; i++){
        NODE node;

        for(int j = 0; j < OPERATION_LENGTH; j++){
          operation_list[j] = action_pattern[i][j];
        }

        if(!is_valid_operation(operation_list)){
          continue;
        }

        /*
        for(int k = 0; k < OPERATION_LENGTH; k++){
          fprintf(stderr,"%d ", operation_list[k]);
        }
        fprintf(stderr,"\n");
        */

        exec_operation(operation_list);
        //check_field();

        node.operation_list = operation_list;
        node.eval = calc_field_eval();

        pque.push(node);

        // 盤面を戻す
        rollback_field();
        // プレイヤーを元の位置に戻る
        rollback_player();
      }

      NODE best_node = pque.top();
      //fprintf(stderr,"best_node.value = %d\n", best_node.eval);
      return best_node.operation_list;
    }

    /**
     * 命令の処理
     */
    bool exec_operation(vector<int> operation_list){
      int size = operation_list.size();
      int cost = 0;
      g_kill_count = 0;

      for(int i = 0; i < size; i++){
        int operation = operation_list[i];

        if(operation == 0){
          // no action
        }else if(operation <= 4){
          assert(operation >= 1);
          attack(operation - 1);
          cost += 4;
        }else if(operation <= 8){
          assert(operation >= 5);
          move(operation - 5);
          cost += 2;
        }else if(operation == 9){
          hide();
          cost += 1;
        }else{
          show_up();
          cost += 1;
        }
      }

      assert(cost <= MAX_COST);

      return true;
    }

    /**
     * フィールドの評価値を更新
     */
    void update_field_value(){
      // 危険値の更新
      for(int player_id = 0; player_id < 6; player_id += 1){
        PLAYER *player = get_player(player_id);
        // 味方の場合は処理を飛ばす
        if(player->group_id == g_group_id) continue;

        if(player->update_at == g_current_turn){
          set_danger_value(player->job, player->y, player->x);
        }
      }
    }

    /**
     * 危険値をセットする
     * @param job 職業
     * @param y 基準となるy座標
     * @param x 基準となるx座標
     */
    void set_danger_value(int job, int y, int x){
      for(int dy = 0; dy < 11; dy++){
        for(int dx = 0; dx < 11; dx++){
          int ny = y + (dy-5);
          int nx = x + (dx-5);

          // フィールド内でかつ、相手の攻撃範囲に含まれていた場合は危険とみなす
          if(is_inside(ny, nx) && PLAYER_KILL_RANGE[job][dy][dx]){
            g_danger_field[ny][nx] += 1;
            assert(g_danger_field[ny][nx] > 0);
            assert(g_danger_field[ny][nx] <= 3);
          }
        }
      }
    }

    /**
     * 危険値を減らす
     * @param job 職業
     * @param y 基準となるy座標
     * @param x 基準となるx座標
     */
    void remove_danger_value(int job, int y, int x){
      for(int dy = 0; dy < 11; dy++){
        for(int dx = 0; dx < 11; dx++){
          int ny = y + (dy-5);
          int nx = x + (dx-5);

          // フィールド内でかつ、相手の攻撃範囲に含まれていた場合は危険とみなす
          if(is_inside(ny, nx) && PLAYER_KILL_RANGE[job][dy][dx]){
            g_danger_field[ny][nx] -= 1;

            assert(0 <= g_danger_field[ny][nx]);
            assert(g_danger_field[ny][nx] <= 2);
          }
        }
      }
    }

    /**
     * 移動を行う
     * @param direct 移動する方向
     */
    bool move(int direct){
      assert(direct >= 0);
      PLAYER *my = get_player(g_playerId);

      int ny = my->y + DY[direct];
      int nx = my->x + DX[direct];

      if(is_inside(ny,nx)){
        my->y = ny;
        my->x = nx;

        return true;
      }else{
        return false;
      }
    }

    /**
     * 攻撃を行う
     * @param direct 攻撃の方向
     */
    bool attack(int direct){
      PLAYER *my = get_player(g_playerId);

      for(int y = 0; y < 9; y++){
        for(int x = 0; x < 9; x++){
          int ny = my->y + (y-4);
          int nx = my->x + (x-4);

          // フィールド内かつ、居館ではなく、攻撃範囲に含まれている
          if(is_inside(ny, nx) && !is_exist_kyokan(ny, nx) && ((PLAYER_ATTACK_RANGE[my->job][y][x] >> direct) & 1)){
            g_field[ny][nx] = g_playerId;
            int killed_id = is_exist_enemy(ny, nx);

            if(killed_id != UNKNOWN){
              g_kill_count += 1;

              assert(g_kill_count <= 3);
              assert(killed_id < MAX_PLAYER_NUM);

              PLAYER *enemy = get_player(killed_id);
              assert(enemy->group_id != g_group_id);
              remove_danger_value(enemy->job, enemy->y, enemy->x);
            }
          }
        }
      }

      return true;
    }

    /*
     * フィールドの状態を確認する
     * 1. 攻撃済みのフィールドに敵がいるかどうか
     */
    void check_field(){
      // 1. 状態が判明している敵で倒したかどうか
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        PLAYER *player = get_player(player_id);

        // 味方の場合は処理を飛ばす
        if(player->group_id == g_group_id) continue;

        // 情報が最新である
        if(player->update_at == g_current_turn){
        }
      }
    }

    /**
     * 潜伏を行う
     * @return (true: 潜伏成功, false: 潜伏失敗)
     */
    bool hide(){
      PLAYER *my = get_player(g_playerId);

      if(can_hide(my->y, my->x)){
        my->status = HIDE;
        return true;
      }else{
        return false;
      }
    }

    /**
     * 顕現を行う
     * @return (true: 顕現成功, false: 顕現失敗)
     */
    bool show_up(){
      PLAYER *my = get_player(g_playerId);
      
      if(can_show_up(my->y, my->x)){
        my->status = NOHIDE;
        return true;
      }else{
        return false;
      }
    }

    /**
     * AIの実行
     */
    void run(){
      while(true){
        if(update_game_data()){
          vector<int> operation_list = action();
          output(operation_list);
        }else{
          break;
        }
        /*
        fprintf(stderr,"---------------------------\n");
        show_field();
        fprintf(stderr,"---------------------------\n");
        */
      }
    }

    /**
     * 指定したIDのプレイヤー情報を取得する
     * @param player_id 取得したいプレイヤーのID
     * @return プレイヤーの情報
     */
    inline PLAYER *get_player(int player_id){
      return &g_playerList[player_id];
    }

    /**
     * 指定したIDの居館を取得
     * @param kyokan_id 取得したい居館のID
     * @return 居館の情報
     */
    inline KYOKAN *get_kyokan(int kyokan_id){
      return &g_kyokanList[kyokan_id];
    }

    /**
     * フィールドの状態を評価する
     * @return 評価値
     */
    double calc_field_eval(){
      PLAYER *my = get_player(g_playerId);

      // チームで見れる視界の数
      int can_view_count = get_visible_cell_count();
      // 自分の領土の数
      int owner_count = 0;
      // 相手チームの領土の数
      int enemy_area_count = 0;
      // 自分のチームの領土の数
      int friend_area_count = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int group_id = get_group_id(y,x);

          if(g_field[y][x] == g_playerId){
            owner_count += 1;
          }else if(group_id != UNKNOWN){
            if(group_id != g_group_id){
              enemy_area_count += 1;
            }else{
              friend_area_count += 1;
            }
          }
        }
      }

      //fprintf(stderr,"(%d, %d) owner_count = %d\n", my->y, my->x, owner_count);

      //fprintf(stderr,"kill_count = %d, friend_area_count = %d, owner_count = %d, enemy_area_count = %d\n", g_kill_count, friend_area_count, owner_count, enemy_area_count);

      int value = 0;

      // 敵に見られている場合はポイントを下げる
      if(g_visibled_field[my->y][my->x] && my->status == NOHIDE){
        value -= 5;
      }

      if(my->job == SPEAR){
        return -30 * g_danger_field[my->y][my->x] + 15 * g_kill_count + (friend_area_count + 2 * owner_count) - enemy_area_count + 0.5 * can_view_count;
      // 職業が剣の場合の評価値
      }else if(my->job == SWORD){
        return -30 * g_danger_field[my->y][my->x] + 15 * g_kill_count + (friend_area_count + 2 * owner_count) - enemy_area_count + 0.5 * can_view_count + value;
      // 職業がマサカリの時の評価値
      }else if(my->job == AX){
        return -20 * g_danger_field[my->y][my->x] + 20 * g_kill_count + (friend_area_count + 2 * owner_count) - enemy_area_count + 0.5 * can_view_count + value;
      }else{
        assert(false);
      }
    }

    /**
     * 命令リストを対戦サーバに送信
     * @param operation_list 命令リスト
     */
    void output(vector<int> operation_list){
      int size = operation_list.size();

      /**
       * 足りない部分は0で埋める
       */
      for(int i = 0; i < OPERATION_LENGTH - size; i++){
        operation_list.push_back(0);
      }

      for(int i = 0; i < OPERATION_LENGTH; i++){
        cout << operation_list[i];

        if(i != 6){
          cout << " ";
        }
      }

      cout << endl;
    }

    /**
     * z座標の取得を行う
     */
    inline int getZ(int y, int x){
      return y * MAX_WIDTH + x;
    }

    /**
     * フィールド内かどうかを調べる
     * @param y 調べたいy座標
     * @param x 調べたいx座標
     * @return (true: フィールド内, false: フィールド外)
     */
    inline bool is_inside(int y, int x){
      return (0 <= y && y < g_height && 0 <= x && x < g_width);
    }

    /**
     * フィールド外かどうかを調べる
     * @param y 調べたいy座標
     * @param x 調べたいx座標
     * @return (true: フィールド外、 false: フィールド内)
     */
    inline bool is_outside(int y, int x){
      return (y < 0 || g_height <= y || x < 0 || g_width <= x);
    }

    /**
     * そのマスに敵がいるかどうかを調べる
     * @param y 調べたいy座標
     * @param x 調べたいx座標
     * @return (UNKNOWN: 敵は居ない, その他: 敵のID)
     */
    int is_exist_enemy(int y, int x){
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        PLAYER *player = get_player(player_id);
        // 味方の場合は処理を飛ばす
        if(player->group_id == g_group_id) continue;

        if(player->y == y && player->x == x && player->update_at == g_current_turn){
          return player->id;
        }
      }

      return UNKNOWN;
    }

    /**
     * そのマスに居館があるかどうかを調べる
     * @param y 調べたいy座標
     * @param x 調べたいx座標
     * @return 居館があるかどうか
     */
    bool is_exist_kyokan(int y, int x){
      for(int kyokan_id = 0; kyokan_id < MAX_PLAYER_NUM; kyokan_id++){
        KYOKAN *kyokan = get_kyokan(kyokan_id);

        if(kyokan->y == y && kyokan->x == x){
          return true;
        }
      }

      return false;
    }

    /**
     * 指定したマスに行動可能かどうかを返す
     * @param y 現在のy座標
     * @param x 現在のx座標
     * @param direct 進みたい方向
     * @param status 潜伏状態かどうか
     */
    bool can_move(int y, int x, int direct, int status){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      // 1. フィールド外であれば移動出来ない
      if(is_outside(ny,nx)){
        //fprintf(stderr,"y = %d, x = %d is outside!\n", ny, nx);
        return false;
      }
      // 2. 潜伏状態でかつ自陣のフィールドでなければ移動出来ない
      if(status == HIDE && get_group_id(ny,nx) != g_group_id){
        //fprintf(stderr,"y = %d, x = %d not my group erea!\n", ny, nx);
        return false;
      }

      return true;
    }

    /**
     * 潜伏できるかどうかを返す
     * @param y 潜伏するy座標
     * @param x 潜伏するx座標
     * @return 潜伏できるかどうかを返す
     */
    inline bool can_hide(int y, int x){
      // 自身のチームの領域あった場合は潜伏出来る
      return (get_group_id(y,x) == g_group_id);
    }

    /**
     * 顕現出来るかどうかを返す
     * @param y 顕現するy座標
     * @param x 顕現するx座標
     * @return 顕現できるかどうかの判定
     */
    inline bool can_show_up(int y, int x){
      for(int playerId = 0; playerId < MAX_PLAYER_NUM; playerId++){
        if(playerId == g_playerId) continue;

        PLAYER *player = get_player(playerId);
        if(player->y == y && player->x == x){
          return false;
        }
      }

      return true;
    }

    /**
     * 有効な命令かどうかを調べる
     *   1. 攻撃はコストオーバ以外で違反にはならない
     *   2. 0は何も行わない命令
     *   3. 攻撃に関してはコストオーバ以外で失敗はないので無視
     */
    bool is_valid_operation(vector<int> operation_list){
      bool valid = true;
      PLAYER *my = get_player(g_playerId);

      for(int i = 0; i < OPERATION_LENGTH; i++){
        int operation = operation_list[i];

        if(operation == 0){
          // Nothing
        }else if(operation <= 4){
          // Nothing
        }else if(operation <= 8){
          int direct = operation - 5;
          valid &= can_move(my->y, my->x, direct, my->status);
        }else if(operation == 9){
          valid &= can_hide(my->y, my->x);
        }else if(operation == 10){
          valid &= can_show_up(my->y, my->x);
        }
      }

      return valid;
    }

    /**
     * 現在の場面を保存する
     */
    void save_field(){
      g_temp_field = g_field;
      g_temp_danger_field = g_danger_field;
    }

    /**
     * 保存していた盤面を元に戻す
     */
    void rollback_field(){
      g_field = g_temp_field;
      g_danger_field = g_temp_danger_field;
    }

    /**
     * 自分の状態を元に戻す
     *   1. 座標を戻す
     *   2. 状態を戻す
     */
    void rollback_player(){
      PLAYER *my = get_player(g_playerId);

      my->y = my->beforeY;
      my->x = my->beforeX;
      my->status = my->beforeStatus;
    }

    /**
     * 指定した座標のグループIDを取得する
     * @param y y座標
     * @param x x座標
     * @return 占領しているグループのID、誰も占領していない or 情報がわからない場合はUNKNOWNを返す
     */
    int get_group_id(int y, int x){
      int value = g_field[y][x];

      if(value == NEUTRAL || value == NOTHING){
        return UNKNOWN;
      }else{
        return value/3;
      }
    }

    /**
     * 敵に見られている範囲を表す
     */
    void set_enemy_sight(){
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        PLAYER *player = get_player(player_id);
        // 味方の場合は処理を飛ばす
        if(player->group_id != g_group_id) continue;
        // 最新の情報でなければ処理を飛ばす
        if(player->update_at != g_current_turn) continue;

        queue<COORD> que;
        map<int, bool> check_list;
        que.push(COORD(player->y, player->x, 0));

        while(!que.empty()){
          COORD coord = que.front(); que.pop();
          int z = getZ(coord.y, coord.x);

          if(check_list[z] || coord.dist > 5) continue;
          check_list[z] = true;

          g_visibled_field[coord.y][coord.x] = true;

          for(int i = 0; i < 4; i++){
            int ny = coord.y + DY[i];
            int nx = coord.x + DX[i];

            if(is_inside(ny, nx)){
              que.push(COORD(ny, nx, coord.dist+1));
            }
          }
        }
      }
    }

    /**
     * 見ることが出来るセルの数を数える
     */
    int get_visible_cell_count(){
      int visible_count = 0;
      map<int, bool> all_check_list;

      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id += 1){
        PLAYER *player = get_player(player_id);
        if(player->group_id != g_group_id) continue;


        queue<COORD> que;
        map<int, bool> check_list;
        que.push(COORD(player->y, player->x, 0));

        while(!que.empty()){
          COORD coord = que.front(); que.pop();
          int z = getZ(coord.y, coord.x);

          if(check_list[z] || coord.dist > 5) continue;
          check_list[z] = true;

          if(!all_check_list[z]){
            all_check_list[z] = true;
            visible_count += 1;
          }

          for(int i = 0; i < 4; i++){
            int ny = coord.y + DY[i];
            int nx = coord.x + DX[i];

            if(is_inside(ny, nx)){
              que.push(COORD(ny, nx, coord.dist+1));
            }
          }
        }
      }

      return visible_count;
    }

    /**
     * 座標間の距離を計算
     */
    inline int calc_dist(int y1, int x1, int y2, int x2){
      return abs(y1-y2) + abs(x1-x2);
    }

    /**
     * フィールドを画面に出力する(デバッグ用)
     */
    void show_field(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_field[y][x] == g_playerId){
            fprintf(stderr,"M");
          }else{
            fprintf(stderr,"%d", g_field[y][x]);
          }
        }
        fprintf(stderr,"\n");
      }
    }
};

int main(){
  SamurAI player;
  player.init();
  player.run();
  return 0;
}
