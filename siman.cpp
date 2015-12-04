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

struct PLAYER{
	int id;				// サムライ番号
	int y;				// 現在のy座標
	int x;				// 現在のx座標
	int homeY;		// 居館のy座標
	int homeX; 		// 居館のx座標
	int status; 	// 潜伏状態かどうか

	PLAYER(int id = UNKNOWN){
		this->id = id;
		this->y = UNKNOWN;
		this->x = UNKNOWN;
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
int g_field[MAX_HEIGHT][MAX_WIDTH];

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
		void updateFieldData(){
			fprintf(stderr,"updateFieldData =>\n");
			// 現在のターンの取得
			scanf("%d", &g_currentTurn);
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
					int cell;
					scanf("%d", &cell);
					g_field[y][x] = cell;
				}
			}
		}

		/**
		 * 行動
		 */
		void move(){
			cout << "0 0 0 0 0 0 0" << endl;
		}

    /**
     * AIの実行
     */
		void run(){
			while(true){
				updateFieldData();
				move();
			}
		}

		PLAYER *getPlayer(int id){
			return &g_playerList[id];
		}
};

int main(){
	SamurAI player;
	player.init();
	player.run();
  return 0;
}
