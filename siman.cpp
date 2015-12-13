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

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

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
  int eval;

  bool operator >(const NODE &e) const{
    return eval < e.eval;
  }    
};

struct PLAYER{
  int id;				// サムライ番号
  int group_id; // グループID
  int y;				// 現在のy座標
  int x;				// 現在のx座標
  int beforeY;  // 前のターンのy座標
  int beforeX;  // 前のターンのx座標
  int job;      // 職種
  int homeY;		// 居館のy座標
  int homeX; 		// 居館のx座標
  int status; 	// 潜伏状態かどうか
  int beforeStatus; // 前回の状態
  int rank;     // 現在のランキング
  int score;    // 現在のスコア

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
int g_groupId;
// 最大ターン数
int g_max_turn;
// フィールドの横幅
int g_width;
// フィールドの縦幅
int g_height;
// 現在のターン
int g_currentTurn;
// 治療期間
int g_cure_period;
// フィールド
//int g_field[MAX_HEIGHT*MAX_WIDTH];
vector< vector<int> > g_field(MAX_HEIGHT, vector<int>(MAX_WIDTH, 0));
// 一時保存用
//int g_temp_field[MAX_HEIGHT*MAX_WIDTH];
vector< vector<int> > g_temp_field(MAX_HEIGHT, vector<int>(MAX_WIDTH, 0));
// 倒した数
int g_kill_count = 0;

const int ACTION_PATTERN_NUM = 41;

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


// プレイヤーの倒せる範囲
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
      scanf("%d %d %d %d %d", &g_max_turn, &g_groupId, &g_playerId, &g_width, &g_height);
      fprintf(stderr,"ready =>\n");

      fprintf(stderr,"max_turn => %d\n", g_max_turn);
      fprintf(stderr,"player id => %d\n", g_playerId);
      fprintf(stderr,"group id => %d\n", g_groupId);
      fprintf(stderr,"width: %d, height: %d\n", g_width, g_height);
      vector< vector<int> > g_field(g_height, vector<int>(g_width, 0));
      vector< vector<int> > g_temp_field(g_height, vector<int>(g_width, 0));

      // 居館の位置を取得（ついでにユーザの初期位置を設定）
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        int homeY;
        int homeX;

        KYOKAN *kyokan = getKyokan(player_id);
        PLAYER *player = getPlayer(player_id);
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

        PLAYER *player = getPlayer(id);
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
    bool updateGameData(){
      //fprintf(stderr,"updateGameData =>\n");
      // 現在のターンの取得
      int status = scanf("%d", &g_currentTurn);

      if(status == -1){
        return false;
      }

      //fprintf(stderr,"currentTurn => %d\n", g_currentTurn);

      // 治療期間の取得
      scanf("%d", &g_cure_period);

      if(g_cure_period > 0){
        fprintf(stderr,"%d: I'm sleepy now...\n", g_cure_period);
      }

      // 各プレイヤーの情報を更新
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int y;
        int x;
        int status;

        PLAYER *player = getPlayer(id);
        scanf("%d %d %d", &x, &y, &status);

        if(status != UNKNOWN){
          player->update(y, x, status);
        }

        //fprintf(stderr,"id: %d, y: %d, x: %d, status: %d\n", id, player->y, player->x, player->status);
        //fprintf(stderr,"input id: %d, y: %d, x: %d, status: %d\n", id, y, x, status);

        //fprintf(stderr,"id: %d, y: %d, x: %d, beforeY = %d, beforeX = %d, status: %d\n", id, player->y, player->x, player->beforeY, player->beforeX, player->status);
      }

      // フィールド情報の更新
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int cell;
          scanf("%d", &cell);
          //g_field[z] = cell;
          g_field[y][x] = cell;
        }
      }

      return true;
    }

    /**
     * 行動
     */
    vector<int> action(){
      vector<int> operation_list(7,0);

      priority_queue< NODE, vector<NODE>, greater<NODE> > pque;

      // 盤面を保存
      save_field();

      //for(int i = 0; i < 10; i++){
      for(int i = 0; i < ACTION_PATTERN_NUM; i++){
        NODE node;

        for(int j = 0; j < 7; j++){
          operation_list[j] = action_pattern[i][j];
        }

        if(!is_valid_operation(operation_list)){
          continue;
        }

        /*
        for(int k = 0; k < 7; k++){
          fprintf(stderr,"%d ", operation_list[k]);
        }
        fprintf(stderr,"\n");
        */

        exec_operation(operation_list);

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
      g_kill_count = 0;

      for(int i = 0; i < size; i++){
        int operation = operation_list[i];

        if(operation == 0){
          // no action
        }else if(operation <= 4){
          assert(operation >= 1);
          attack(operation - 1);
        }else if(operation <= 8){
          assert(operation >= 5);
          move(operation - 5);
        }else if(operation == 9){
          hide();
        }else{
          show_up();
        }
      }

      return true;
    }

    /**
     * 移動
     */
    bool move(int direct){
      PLAYER *my = getPlayer(g_playerId);

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
     * 攻撃
     */
    bool attack(int direct){
      PLAYER *my = getPlayer(g_playerId);

      for(int y = 0; y < 9; y++){
        for(int x = 0; x < 9; x++){
          int ny = my->y + (y-4);
          int nx = my->x + (x-4);
          int nz = getZ(ny,nx);

          // フィールド内かつ、居館ではなく、攻撃範囲に含まれている
          if(is_inside(ny, nx) && !is_exist_kyokan(ny, nx) && ((PLAYER_ATTACK_RANGE[my->job][y][x] >> direct) & 1)){
            //fprintf(stderr,"(%d, %d) checked\n", ny, nx);
            //g_field[nz] = g_playerId;
            g_field[ny][nx] = g_playerId;

            if(is_exist_enemy(ny, nx)){
              g_kill_count += 1;
            }
          }
        }
      }

      return true;
    }

    /**
     * 潜伏
     */
    bool hide(){
      PLAYER *my = getPlayer(g_playerId);

      if(can_hide(my->y, my->x)){
        my->status = HIDE;
        return true;
      }else{
        return false;
      }
    }

    /**
     * 顕現
     */
    bool show_up(){
      PLAYER *my = getPlayer(g_playerId);
      
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
        if(updateGameData()){
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

    /*
     * 指定したIDのプレイヤー情報を取得する
     */
    PLAYER *getPlayer(int id){
      return &g_playerList[id];
    }

    /*
     * 指定したIDの居館を取得
     */
    KYOKAN *getKyokan(int id){
      return &g_kyokanList[id];
    }

    /*
     * フィールドの状態を評価する
     */
    int calc_field_eval(){
      PLAYER *my = getPlayer(g_playerId);

      // チームで見れる視界の数
      int can_view_count = 0;
      // 自分の領土の数
      int owner_count = 0;
      // 相手チームの領土の数
      int enemy_area_count = 0;
      // 自分のチームの領土の数
      int friend_area_count = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);
          int group_id = get_group_id(y,x);

          if(g_field[y][x] == g_playerId){
            owner_count += 1;
          }

          if(group_id != UNKNOWN && group_id != g_groupId){
            enemy_area_count += 1;
          }else if(group_id != UNKNOWN && group_id == g_groupId){
            friend_area_count += 1;
          }
        }
      }

      //fprintf(stderr,"(%d, %d) owner_count = %d\n", my->y, my->x, owner_count);

      //fprintf(stderr,"kill_count = %d, friend_area_count = %d, owner_count = %d, enemy_area_count = %d\n", g_kill_count, friend_area_count, owner_count, enemy_area_count);

      return 10 * g_kill_count + (friend_area_count + 2 * owner_count) - enemy_area_count;
    }

    /*
     * 命令リストを対戦サーバに通知
     */
    void output(vector<int> operation_list){
      int size = operation_list.size();

      /*
       * 足りない部分は0で埋める
       */
      for(int i = 0; i < 7 - size; i++){
        operation_list.push_back(0);
      }

      for(int i = 0; i < 7; i++){
        cout << operation_list[i];

        if(i != 6){
          cout << " ";
        }
      }

      cout << endl;
    }

    /*
     * z座標の取得を行う
     */
    inline int getZ(int y, int x){
      return y * MAX_WIDTH + x;
    }

    /*
     * フィールド内に存在しているかどうかを調べる
     */
    inline bool is_inside(int y, int x){
      return (0 <= y && y < g_height && 0 <= x && x < g_width);
    }

    /*
     * フィールドの範囲外かどうかを調べる
     */
    inline bool is_outside(int y, int x){
      return (y < 0 || g_height <= y || x < 0 || g_width <= x);
    }

    /*
     * そのマスに敵がいるかどうかを調べる
     */
    bool is_exist_enemy(int y, int x){
      for(int player_id = 0; player_id < MAX_PLAYER_NUM; player_id++){
        PLAYER *player = getPlayer(player_id);
        if(player->group_id == g_groupId) continue;

        if(player->y == y && player->x == x){
          return true;
        }
      }

      return false;
    }

    /*
     * そのマスに居館があるかどうかを調べる
     */
    bool is_exist_kyokan(int y, int x){
      for(int kyokan_id = 0; kyokan_id < MAX_PLAYER_NUM; kyokan_id++){
        KYOKAN *kyokan = getKyokan(kyokan_id);

        if(kyokan->y == y && kyokan->x == x){
          return true;
        }
      }

      return false;
    }

    /*
     * 行動可能かどうかを返す
     */
    bool can_move(int y, int x, int direct, int status){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      if(is_outside(ny,nx)){
        //fprintf(stderr,"y = %d, x = %d is outside!\n", ny, nx);
        return false;
      }
      if(status == HIDE && get_group_id(ny,nx)!= g_groupId){
        //fprintf(stderr,"y = %d, x = %d not my group erea!\n", ny, nx);
        return false;
      }

      return true;
    }

    /*
     * 潜伏できるかどうかを返す
     */
    inline bool can_hide(int y, int x){
      int z = getZ(y,x);

      // いずれかのサムライの陣地であり、かつ自身のチームの領域あった場合
      if(g_field[y][x] <= 7 && g_field[y][x]/3 == g_groupId){
        return true;
      }else{
        return false;
      }
    }

    /*
     * 顕現出来るかどうかを返す
     */
    inline bool can_show_up(int y, int x){
      for(int playerId = 0; playerId < MAX_PLAYER_NUM; playerId++){
        if(playerId == g_playerId) continue;

        PLAYER *player = getPlayer(playerId);
        if(player->y == y && player->x == x){
          return false;
        }
      }

      return true;
    }

    /*
     * 有効な命令かどうかを調べる
     *   1. 攻撃はコストオーバ以外で違反にはならない
     */
    bool is_valid_operation(vector<int> operation_list){
      bool valid = true;
      PLAYER *my = getPlayer(g_playerId);

      for(int i = 0; i < 7; i++){
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

    /*
     * 現在の場面を保存する
     */
    void save_field(){
      g_temp_field = g_field;
      //memcpy(g_temp_field, g_field, sizeof(g_field));
    }

    /*
     * 保存していた盤面を戻す
     */
    void rollback_field(){
      g_field = g_temp_field;
      //memcpy(g_field, g_temp_field, sizeof(g_temp_field));
    }

    /*
     * Rollback Player
     */
    void rollback_player(){
      PLAYER *my = getPlayer(g_playerId);

      my->y = my->beforeY;
      my->x = my->beforeX;
      my->status = my->beforeStatus;
    }

    /*
     * 
     */
    int get_group_id(int y, int x){
      int value = g_field[y][x];

      if(value == 8 || value == 9){
        return UNKNOWN;
      }else{
        return value/3;
      }
    }

    void show_field(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);

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
