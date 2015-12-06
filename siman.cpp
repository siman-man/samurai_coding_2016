#include <iostream>
#include <vector>
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

const int UNKNOWN = -1;
const int SPEAR = 0;
const int SWORD = 1;
const int AX = 2;

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

struct PLAYER{
  int id;				// サムライ番号
  int y;				// 現在のy座標
  int x;				// 現在のx座標
  int job;      // 職種
  int homeY;		// 居館のy座標
  int homeX; 		// 居館のx座標
  int status; 	// 潜伏状態かどうか

  PLAYER(int id = UNKNOWN){
    this->id = id;
    this->y = UNKNOWN;
    this->x = UNKNOWN;
    this->job = id%3;
    this->homeY = UNKNOWN;
    this->homeX = UNKNOWN;
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
int g_curePeriod;
// フィールド
int g_field[MAX_HEIGHT*MAX_WIDTH];

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
  {1, 5, 0, 0, 0, 0, 0},
  {1, 5, 0, 0, 0, 0, 0},
  {1, 5, 0, 0, 0, 0, 0},
  {2, 6, 0, 0, 0, 0, 0},
  {2, 6, 0, 0, 0, 0, 0},
  {2, 6, 0, 0, 0, 0, 0},
  {2, 6, 0, 0, 0, 0, 0},
  {3, 7, 0, 0, 0, 0, 0},
  {3, 7, 0, 0, 0, 0, 0},
  {3, 7, 0, 0, 0, 0, 0},
  {3, 7, 0, 0, 0, 0, 0},
  {4, 8, 0, 0, 0, 0, 0},
  {4, 8, 0, 0, 0, 0, 0},
  {4, 8, 0, 0, 0, 0, 0},
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
      scanf("%d %d %d %d %d", &g_max_turn, &g_playerId, &g_groupId, &g_width, &g_height);
      fprintf(stderr,"ready =>\n");

      fprintf(stderr,"max_turn => %d\n", g_max_turn);
      fprintf(stderr,"player id => %d\n", g_playerId);
      fprintf(stderr,"group id => %d\n", g_groupId);
      fprintf(stderr,"width: %d, height: %d\n", g_width, g_height);

      // 居館の位置を取得（ついでにユーザの初期位置を設定）
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int homeY;
        int homeX;

        PLAYER player(id);
        scanf("%d %d", &homeY, &homeX);

        // プレイヤーの初期位置を初期化
        player.setHomePosition(homeY, homeX);
      }

      // 各プレイヤーの戦績を取得
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int rank;
        int score;

        PLAYER *player = getPlayer(id);
        scanf("%d %d", &rank, &score);

        fprintf(stderr,"id: %d, rank: %d, score: %d\n", id, rank, score);
      }

      // 準備完了の通知をサーバ側に足して行う
      cout << 0 << endl;
    }

    /**
     * フィールド情報の更新を行う
     */
    bool updateGameData(){
      fprintf(stderr,"updateGameData =>\n");
      // 現在のターンの取得
      int status = scanf("%d", &g_currentTurn);

      if(status == -1){
        return false;
      }

      fprintf(stderr,"currentTurn => %d\n", g_currentTurn);

      // 治療期間の取得
      scanf("%d", &g_curePeriod);
      fprintf(stderr,"curePeriod => %d\n", g_curePeriod);

      // 各プレイヤーの情報を更新
      for(int id = 0; id < MAX_PLAYER_NUM; id++){
        int y;
        int x;
        int status;

        PLAYER *player = getPlayer(id);
        scanf("%d %d %d", &y, &x, &status);

        fprintf(stderr,"id: %d, y: %d, x: %d, status: %d\n", id, y, x, status);

        player->update(y, x, status);
      }

      // フィールド情報の更新
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);
          int cell;
          scanf("%d", &cell);
          g_field[z] = cell;
        }
      }

      return true;
    }

    /**
     * 行動
     */
    vector<int> action(){
      vector<int> operation_list(7,0);
      operation_list[1] = 1;
      return operation_list;
    }

    /**
     * 攻撃
     */
    void attack(int direct){
      PLAYER *my = getPlayer(g_playerId);

      for(int y = 0; y < 9; y++){
        for(int x = 0; x < 9; x++){
          int ny = my->y + (y-4);
          int nx = my->x + (x-4);
          int nz = getZ(ny,nx);

          // フィールド内かつ、攻撃範囲に含まれている
          if(isInside(ny, nx) && (PLAYER_ATTACK_RANGE[my->job][y][x] >> direct) & 1){
            g_field[nz] = g_playerId;
          }
        }
      }
    }

    /**
     * AIの実行
     */
    void run(){
      while(true){
        if(updateGameData()){
          //vector<int> operation_list = random_move();
          vector<int> operation_list = action();
          output(operation_list);
        }else{
          break;
        }
        showField();
      }
    }

    /*
     * 指定したIDのプレイヤー情報を取得する
     */
    PLAYER *getPlayer(int id){
      return &g_playerList[id];
    }

    /*
     * ランダムに移動する
     */
    vector<int> random_move(){
      vector<int> direct_list;

      for(int i = 0; i < 3; i++){
        int d = xor128()%4 + 5;
        direct_list.push_back(d);
      }

      return direct_list;
    }

    /*
     * フィールドの状態を評価する
     */
    int calc_field_eval(){
      // チームで見れる視界の数
      int can_view_count = 0;
      // 自分の領土の数
      int owner_count = 0;

      return (can_view_count + owner_count);
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
    inline bool isInside(int y, int x){
      return (0 <= y && y < g_height && 0 <= x && x < g_width);
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

    void showField(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);

          if(g_field[z] == g_playerId){
            fprintf(stderr,"M");
          }else{
            fprintf(stderr,"%d", g_field[z]);
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
