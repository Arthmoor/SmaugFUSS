/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.8 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *                            Chess Game Module                             *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "mud.h"
#include "chess.h"
#include "imc.h"

#define WHITE_BACKGROUND ""
#define BLACK_BACKGROUND ""
#define WHITE_FOREGROUND ""
#define BLACK_FOREGROUND ""

const char *big_pieces[MAX_PIECES][2] = {
   {"%s       ",
    "%s       "},
   {"%s  (-)  ",
    "%s  -|-  "},
   {"%s  ###  ",
    "%s  { }  "},
   {"%s  /-*- ",
    "%s / /   "},
   {"%s  () + ",
    "%s  {}-| "},
   {"%s   @   ",
    "%s  /+\\  "},
   {"%s  ^^^^^^  ",
    "%s  {@}  "},
   {"%s  [-]  ",
    "%s  -|-  "},
   {"%s  ###  ",
    "%s  [ ]  "},
   {"%s  /-*- ",
    "%s / /   "},
   {"%s  [] + ",
    "%s  {}-| "},
   {"%s   #   ",
    "%s  /+\\  "},
   {"%s  ^^^^^^  ",
    "%s  [#]  "}
};

const char small_pieces[MAX_PIECES + 1] = " prnbqkPRNBQK";

static char *print_big_board( CHAR_DATA * ch, GAME_BOARD_DATA * board )
{
   static char retbuf[MAX_STRING_LENGTH * 2];
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   char s1[16], s2[16];
   int x, y;

   sprintf( s1, "&Y&W" );
   sprintf( s2, "&z&z" );

   sprintf( retbuf, WHITE_FOREGROUND "\n\r&g     1      2      3      4      5      6      7      8\n\r" );

   for( x = 0; x < 8; x++ )
   {
      strcat( retbuf, "  " );
      for( y = 0; y < 8; y++ )
      {
         sprintf( buf, "%s%s",
                  x % 2 == 0 ? ( y % 2 == 0 ? BLACK_BACKGROUND : WHITE_BACKGROUND ) :
                  ( y % 2 == 0 ? WHITE_BACKGROUND : BLACK_BACKGROUND ), big_pieces[board->board[x][y]][0] );
         sprintf( buf2, buf, IS_WHITE( board->board[x][y] ) ? s1 : s2 );
         strcat( retbuf, buf2 );
      }
      strcat( retbuf, BLACK_BACKGROUND "\n\r" );

      sprintf( buf, WHITE_FOREGROUND "&g%c ", 'A' + x );
      strcat( retbuf, buf );
      for( y = 0; y < 8; y++ )
      {
         sprintf( buf, "%s%s",
                  x % 2 == 0 ? ( y % 2 == 0 ? BLACK_BACKGROUND : WHITE_BACKGROUND ) :
                  ( y % 2 == 0 ? WHITE_BACKGROUND : BLACK_BACKGROUND ), big_pieces[board->board[x][y]][1] );
         sprintf( buf2, buf, IS_WHITE( board->board[x][y] ) ? s1 : s2 );
         strcat( retbuf, buf2 );
      }
      strcat( retbuf, BLACK_BACKGROUND "\n\r" );
   }

   return ( retbuf );
}

static void init_board( GAME_BOARD_DATA * board )
{
   int x, y;
   for( x = 0; x < 8; x++ )
      for( y = 0; y < 8; y++ )
         board->board[x][y] = 0;
   board->board[0][0] = WHITE_ROOK;
   board->board[0][1] = WHITE_KNIGHT;
   board->board[0][2] = WHITE_BISHOP;
   board->board[0][3] = WHITE_QUEEN;
   board->board[0][4] = WHITE_KING;
   board->board[0][5] = WHITE_BISHOP;
   board->board[0][6] = WHITE_KNIGHT;
   board->board[0][7] = WHITE_ROOK;
   for( x = 0; x < 8; x++ )
      board->board[1][x] = WHITE_PAWN;
   for( x = 0; x < 8; x++ )
      board->board[6][x] = BLACK_PAWN;
   board->board[7][0] = BLACK_ROOK;
   board->board[7][1] = BLACK_KNIGHT;
   board->board[7][2] = BLACK_BISHOP;
   board->board[7][3] = BLACK_QUEEN;
   board->board[7][4] = BLACK_KING;
   board->board[7][5] = BLACK_BISHOP;
   board->board[7][6] = BLACK_KNIGHT;
   board->board[7][7] = BLACK_ROOK;
   board->player1 = NULL;
   board->player2 = NULL;
   board->turn = 0;
   board->type = TYPE_LOCAL;
}

static bool find_piece( GAME_BOARD_DATA * board, int *x, int *y, int piece )
{
   int a, b;

   for( a = 0; a < 8; a++ )
   {
      for( b = 0; b < 8; b++ )
         if( board->board[a][b] == piece )
            break;
      if( board->board[a][b] == piece )
         break;
   }
   *x = a;
   *y = b;
   if( board->board[a][b] == piece )
      return TRUE;
   return FALSE;
}

#define SAME_COLOR(x1,y1,x2,y2)	\
    ((IS_WHITE(board->board[x1][y1]) && IS_WHITE(board->board[x2][y2])) || \
    (IS_BLACK(board->board[x1][y1]) && IS_BLACK(board->board[x2][y2])))

static bool king_in_check( GAME_BOARD_DATA * board, int piece )
{
   int x = 0, y = 0, l, m;

   if( piece != WHITE_KING && piece != BLACK_KING )
      return FALSE;

   if( !find_piece( board, &x, &y, piece ) )
      return FALSE;

   if( x < 0 || y < 0 || x > 7 || y > 7 )
      return FALSE;

   /*
    * pawns 
    */
   if( IS_WHITE( piece ) && x < 7 &&
       ( ( y > 0 && IS_BLACK( board->board[x + 1][y - 1] ) ) || ( y < 7 && IS_BLACK( board->board[x + 1][y + 1] ) ) ) )
      return TRUE;
   else if( IS_BLACK( piece ) && x > 0 &&
            ( ( y > 0 && IS_WHITE( board->board[x - 1][y - 1] ) ) || ( y < 7 && IS_WHITE( board->board[x - 1][y + 1] ) ) ) )
      return TRUE;
   /*
    * knights 
    */
   if( x - 2 >= 0 && y - 1 >= 0 &&
       ( ( board->board[x - 2][y - 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 2][y - 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x - 2 >= 0 && y + 1 < 8 &&
       ( ( board->board[x - 2][y + 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 2][y + 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x - 1 >= 0 && y - 2 >= 0 &&
       ( ( board->board[x - 1][y - 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 1][y - 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x - 1 >= 0 && y + 2 < 8 &&
       ( ( board->board[x - 1][y + 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x - 1][y + 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x + 1 < 8 && y - 2 >= 0 &&
       ( ( board->board[x + 1][y - 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 1][y - 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x + 1 < 8 && y + 2 < 8 &&
       ( ( board->board[x + 1][y + 2] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 1][y + 2] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   if( x + 2 < 8 && y - 1 >= 0 &&
       ( ( board->board[x + 2][y - 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 2][y - 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;
   if( x + 2 < 8 && y + 1 < 8 &&
       ( ( board->board[x + 2][y + 1] == BLACK_KNIGHT && IS_WHITE( board->board[x][y] ) ) ||
         ( board->board[x + 2][y + 1] == WHITE_KNIGHT && IS_BLACK( board->board[x][y] ) ) ) )
      return TRUE;

   /*
    * horizontal/vertical long distance 
    */
   for( l = x + 1; l < 8; l++ )
      if( board->board[l][y] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, y ) )
            break;
         if( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
             board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( l = x - 1; l >= 0; l-- )
      if( board->board[l][y] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, y ) )
            break;
         if( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
             board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( m = y + 1; m < 8; m++ )
      if( board->board[x][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, x, m ) )
            break;
         if( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
             board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
            return TRUE;
         break;
      }
   for( m = y - 1; m >= 0; m-- )
      if( board->board[x][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, x, m ) )
            break;
         if( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
             board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
            return TRUE;
         break;
      }
   /*
    * diagonal long distance 
    */
   for( l = x + 1, m = y + 1; l < 8 && m < 8; l++, m++ )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x - 1, m = y + 1; l >= 0 && m < 8; l--, m++ )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x + 1, m = y - 1; l < 8 && m >= 0; l++, m-- )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   for( l = x - 1, m = y - 1; l >= 0 && m >= 0; l--, m-- )
      if( board->board[l][m] != NO_PIECE )
      {
         if( SAME_COLOR( x, y, l, m ) )
            break;
         if( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
             board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
            return TRUE;
         break;
      }
   return FALSE;
}

static bool king_in_checkmate( GAME_BOARD_DATA * board, int piece )
{
   int x = 0, y = 0, dx, dy, sk = 0;

   if( piece != WHITE_KING && piece != BLACK_KING )
      return FALSE;

   if( !find_piece( board, &x, &y, piece ) )
      return FALSE;

   if( x < 0 || y < 0 || x > 7 || y > 7 )
      return FALSE;

   if( !king_in_check( board, board->board[x][y] ) )
      return FALSE;

   dx = x + 1;
   dy = y + 1;
   if( dx < 8 && dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y + 1;
   if( dx >= 0 && dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x + 1;
   dy = y - 1;
   if( dx < 8 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y - 1;
   if( dx >= 0 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x;
   dy = y + 1;
   if( dy < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x;
   dy = y - 1;
   if( dy >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x + 1;
   dy = y;
   if( dx < 8 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   dx = x - 1;
   dy = y;
   if( dx >= 0 && board->board[dx][dy] == NO_PIECE )
   {
      sk = board->board[dx][dy] = board->board[x][y];
      board->board[x][y] = NO_PIECE;
      if( !king_in_check( board, sk ) )
      {
         board->board[x][y] = sk;
         board->board[dx][dy] = NO_PIECE;
         return FALSE;
      }
      board->board[x][y] = sk;
      board->board[dx][dy] = NO_PIECE;
   }
   return TRUE;
}

static int is_valid_move( CHAR_DATA * ch, GAME_BOARD_DATA * board, int x, int y, int dx, int dy )
{
   if( dx < 0 || dy < 0 || dx > 7 || dy > 7 )
      return MOVE_OFFBOARD;

   if( board->board[x][y] == NO_PIECE )
      return MOVE_INVALID;

   if( x == dx && y == dy )
      return MOVE_INVALID;

   if( IS_WHITE( board->board[x][y] ) && !str_cmp( board->player1, ch->name ) )
      return MOVE_WRONGCOLOR;
   if( IS_BLACK( board->board[x][y] ) && ( !str_cmp( board->player2, ch->name ) || !ch ) )
      return MOVE_WRONGCOLOR;

   switch ( board->board[x][y] )
   {
      case WHITE_PAWN:
      case BLACK_PAWN:
         if( IS_WHITE( board->board[x][y] ) &&
             dx == x + 2 && x == 1 && dy == y && board->board[dx][dy] == NO_PIECE && board->board[x + 1][dy] == NO_PIECE )
            return MOVE_OK;
         else if( IS_BLACK( board->board[x][y] ) &&
                  dx == x - 2 && x == 6 && dy == y &&
                  board->board[dx][dy] == NO_PIECE && board->board[x - 1][dy] == NO_PIECE )
            return MOVE_OK;
         if( IS_WHITE( board->board[x][y] ) && dx != x + 1 )
            return MOVE_INVALID;
         else if( IS_BLACK( board->board[x][y] ) && dx != x - 1 )
            return MOVE_INVALID;
         if( dy != y && dy != y - 1 && dy != y + 1 )
            return MOVE_INVALID;
         if( dy == y )
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_OK;
            else if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            else
               return MOVE_BLOCKED;
         }
         else
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_INVALID;
            else if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            else if( board->board[dx][dy] != BLACK_KING && board->board[dx][dy] != WHITE_KING )
               return MOVE_TAKEN;
            else
               return MOVE_INVALID;
         }
         break;
      case WHITE_ROOK:
      case BLACK_ROOK:
      {
         int cnt;

         if( dx != x && dy != y )
            return MOVE_INVALID;

         if( dx == x )
         {
            for( cnt = y; cnt != dy; )
            {
               if( cnt != y && board->board[x][cnt] != NO_PIECE )
                  return MOVE_BLOCKED;
               if( dy > y )
                  cnt++;
               else
                  cnt--;
            }
         }
         else if( dy == y )
         {
            for( cnt = x; cnt != dx; )
            {
               if( cnt != x && board->board[cnt][y] != NO_PIECE )
                  return MOVE_BLOCKED;
               if( dx > x )
                  cnt++;
               else
                  cnt--;
            }
         }

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_KNIGHT:
      case BLACK_KNIGHT:
         if( ( dx == x - 2 && dy == y - 1 ) ||
             ( dx == x - 2 && dy == y + 1 ) ||
             ( dx == x - 1 && dy == y - 2 ) ||
             ( dx == x - 1 && dy == y + 2 ) ||
             ( dx == x + 1 && dy == y - 2 ) ||
             ( dx == x + 1 && dy == y + 2 ) || ( dx == x + 2 && dy == y - 1 ) || ( dx == x + 2 && dy == y + 1 ) )
         {
            if( board->board[dx][dy] == NO_PIECE )
               return MOVE_OK;
            if( SAME_COLOR( x, y, dx, dy ) )
               return MOVE_SAMECOLOR;
            return MOVE_TAKEN;
         }
         return MOVE_INVALID;
         break;
      case WHITE_BISHOP:
      case BLACK_BISHOP:
      {
         int l, m, blocked = FALSE;

         if( dx == x || dy == y )
            return MOVE_INVALID;

         l = x;
         m = y;

         while( 1 )
         {
            if( dx > x )
               l++;
            else
               l--;
            if( dy > y )
               m++;
            else
               m--;
            if( l > 7 || m > 7 || l < 0 || m < 0 )
               return MOVE_INVALID;
            if( l == dx && m == dy )
               break;
            if( board->board[l][m] != NO_PIECE )
               blocked = TRUE;
         }
         if( l != dx || m != dy )
            return MOVE_INVALID;

         if( blocked )
            return MOVE_BLOCKED;

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_QUEEN:
      case BLACK_QUEEN:
      {
         int l, m, blocked = FALSE;

         l = x;
         m = y;

         while( 1 )
         {
            if( dx > x )
               l++;
            else if( dx < x )
               l--;
            if( dy > y )
               m++;
            else if( dy < y )
               m--;
            if( l > 7 || m > 7 || l < 0 || m < 0 )
               return MOVE_INVALID;
            if( l == dx && m == dy )
               break;
            if( board->board[l][m] != NO_PIECE )
               blocked = TRUE;
         }
         if( l != dx || m != dy )
            return MOVE_INVALID;

         if( blocked )
            return MOVE_BLOCKED;

         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;

         if( !SAME_COLOR( x, y, dx, dy ) )
            return MOVE_TAKEN;

         return MOVE_SAMECOLOR;
      }
         break;
      case WHITE_KING:
      case BLACK_KING:
      {
         int sp, sk;
         if( dx > x + 1 || dx < x - 1 || dy > y + 1 || dy < y - 1 )
            return MOVE_INVALID;
         sk = board->board[x][y];
         sp = board->board[dx][dy];
         board->board[x][y] = sp;
         board->board[dx][dy] = sk;
         if( king_in_check( board, sk ) )
         {
            board->board[x][y] = sk;
            board->board[dx][dy] = sp;
            return MOVE_CHECK;
         }
         board->board[x][y] = sk;
         board->board[dx][dy] = sp;
         if( board->board[dx][dy] == NO_PIECE )
            return MOVE_OK;
         if( SAME_COLOR( x, y, dx, dy ) )
            return MOVE_SAMECOLOR;
         return MOVE_TAKEN;
      }
         break;
      default:
         bug( "Invaild piece: %d", board->board[x][y] );
         return MOVE_INVALID;
   }

   if( ( IS_WHITE( board->board[x][y] ) && IS_WHITE( board->board[dx][dy] ) ) ||
       ( IS_BLACK( board->board[x][y] ) && IS_BLACK( board->board[dx][dy] ) ) )
      return MOVE_SAMECOLOR;

   return MOVE_OK;
}

#undef SAME_COLOR

#ifdef IMC
void imc_send_chess( const char *from, const char *to, const char *argument )
{
   IMC_PACKET *p;

   if( this_imcmud->state < IMC_ONLINE )
      return;

   p = imc_newpacket( from, "chess", to );
   imc_addtopacket( p, "text=%s", argument );
   imc_write_packet( p );

   /*
    * setdata(&out, getdata(ch));
    * 
    * imc_sncpy(out.to, to, IMC_NAME_LENGTH);
    * strcpy(out.type, "chess");
    * imc_addkey(&out.data, "text", argument);
    * 
    * imc_send(&out);
    * imc_freedata(&out.data); 
    */
}

PFUN( imc_recv_chess )
{
   CHAR_DATA *victim;
   char txt[LGST], buf[LGST];

   imc_getData( txt, "text", packet );

   /*
    * Chess packets must have a specific destination 
    */
   if( !str_cmp( q->to, "*" ) )
      return;

   if( !( victim = imc_find_user( imc_nameof( q->to ) ) ) )
   {
      if( !str_cmp( txt, "stop" ) )
         return;

      snprintf( buf, LGST, "%s is not here.", imc_nameof( q->to ) );
      imc_send_tell( "*", q->from, buf, 1 );
      return;
   }

   if( !victim->pcdata->game_board )
   {
      if( !str_cmp( txt, "stop" ) )
         return;

      snprintf( buf, LGST, "%s is not ready to be joined in a game.", imc_nameof( q->to ) );
      imc_send_tell( "*", q->from, buf, 1 );
      imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
      return;
   }

   if( !str_cmp( txt, "start" ) )
   {
      if( victim->pcdata->game_board->player2 != NULL )
      {
         snprintf( buf, LGST, "%s is already playing a game.", imc_nameof( q->to ) );
         imc_send_tell( "*", q->from, buf, 1 );
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
         return;
      }
      victim->pcdata->game_board->player2 = str_dup( q->from );
      victim->pcdata->game_board->turn = 0;
      victim->pcdata->game_board->type = TYPE_IMC;
      ch_printf( victim, "%s has joined your game.\r\n", q->from );
      imc_send_chess( victim->name, q->from, "accepted" );
      return;
   }

   if( !str_cmp( txt, "accepted" ) )
   {
      if( !victim->pcdata->game_board ||
          victim->pcdata->game_board->player2 == NULL ||
          victim->pcdata->game_board->type != TYPE_IMC || str_cmp( victim->pcdata->game_board->player2, q->from ) )
      {
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from, "stop" );
         return;
      }
      ch_printf( victim, "You have joined %s in a game.\r\n", q->from );
      if( victim->pcdata->game_board->player2 )
         DISPOSE( victim->pcdata->game_board->player2 );
      victim->pcdata->game_board->player2 = str_dup( q->from );
      victim->pcdata->game_board->turn = 1;
      return;
   }

   if( !str_cmp( txt, "stop" ) )
   {
      ch_printf( victim, "%s has stopped the game.\r\n", q->from );
      free_game( victim->pcdata->game_board );
      return;
   }

   if( !str_cmp( txt, "invalidmove" ) )
   {
      send_to_char( "You have issued an invalid move according to the other mud.\r\n", victim );
      interpret( victim, "chess stop" );
      return;
   }

   if( !str_cmp( txt, "moveok" ) )
   {
      send_to_char( "The other mud has accepted your move.\r\n", victim );
      return;
   }

   if( !str_prefix( "move", txt ) )
   {
      char a, b;
      int x, y, dx, dy, ret;

      a = b = ' ';
      x = y = dx = dy = -1;

      if( sscanf( txt, "move %c%d %c%d", &a, &y, &b, &dy ) != 4 ||
          a < '0' || a > '7' || b < '0' || b > '7' || y < 0 || y > 7 || dy < 0 || dy > 7 )
      {
         imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from,
                         "invalidmove" );
         return;
      }

      x = a - '0';
      dx = b - '0';
      x = ( 7 - x );
      y = ( 7 - y );
      dx = ( 7 - dx );
      dy = ( 7 - dy );
      log_printf( "%d, %d -> %d, %d", x, y, dx, dy );
      ret = is_valid_move( NULL, victim->pcdata->game_board, x, y, dx, dy );
      if( ret == MOVE_OK || ret == MOVE_TAKEN )
      {
         GAME_BOARD_DATA *board;
         int piece, destpiece;

         board = victim->pcdata->game_board;
         piece = board->board[x][y];
         destpiece = board->board[dx][dy];
         board->board[dx][dy] = piece;
         board->board[x][y] = NO_PIECE;

         if( king_in_check( board, IS_WHITE( board->board[dx][dy] ) ? WHITE_KING : BLACK_KING ) &&
             ( board->board[dx][dy] != WHITE_KING && board->board[dx][dy] != BLACK_KING ) )
         {
            board->board[dx][dy] = destpiece;
            board->board[x][y] = piece;
         }
         else
         {
            board->turn++;
            imc_send_chess( board->player1 ? board->player1 : NULL, q->from, "moveok" );
            return;
         }
      }
      imc_send_chess( victim->pcdata->game_board->player1 ? victim->pcdata->game_board->player1 : NULL, q->from,
                      "invalidmove" );
      return;
   }

   log_printf( "Unknown chess command from: %s, %s", q->from, txt );
}
#endif

void init_chess( void )
{
#ifdef IMC
   imc_register_packet_handler( "chess", imc_recv_chess );
#endif
}

void free_game( GAME_BOARD_DATA * board )
{
   if( !board )
      return;

#ifdef IMC
   if( board->type == TYPE_IMC )
   {
      imc_send_chess( board->player1 ? board->player1 : NULL, board->player2, "stop" );
      STRFREE( board->player2 );
   }
#endif

   if( board->player1 )
   {
      if( CHAR_DATA *ch = get_char_world( supermob, board->player1 ) ) // Added for bugfix - Findecano 23/11/07
      {
         ch_printf( ch, "The game has been stopped at %d total moves.\r\n", board->turn );
         ch->pcdata->game_board = NULL;
      }
   }

   if( board->player2 )
   {
      if( CHAR_DATA *ch = get_char_world( supermob, board->player2 ) ) // Added for bugfix - Findecano 23/11/07
      {
         ch_printf( ch, "The game has been stopped at %d total moves.\r\n", board->turn );
         ch->pcdata->game_board = NULL;
      }
   }
   STRFREE( board->player1 );
   STRFREE( board->player2 );
   DISPOSE( board );
}

void do_chess( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't be in chess games.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "begin" ) )
   {
      GAME_BOARD_DATA *board;

      if( ch->pcdata->game_board )
      {
         send_to_char( "You are already in a chess match.\r\n", ch );
         return;
      }

      CREATE( board, GAME_BOARD_DATA, 1 );
      init_board( board );
      ch->pcdata->game_board = board;
      ch->pcdata->game_board->player1 = QUICKLINK( ch->name );
      send_to_char( "You have started a game of chess.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "join" ) )
   {
      GAME_BOARD_DATA *board = NULL;
      CHAR_DATA *vch;
      char arg2[MAX_INPUT_LENGTH];

      if( ch->pcdata->game_board )
      {
         send_to_char( "You are already in a game of chess.\r\n", ch );
         return;
      }

      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         send_to_char( "Join whom in a chess match?\r\n", ch );
         return;
      }

#ifdef IMC
      if( strstr( arg2, "@" ) )
      {
         if( !str_cmp( imc_mudof( arg2 ), this_imcmud->localname ) )
         {
            send_to_char( "You cannot join IMC chess on the local mud!\r\n", ch );
            return;
         }

         if( !str_cmp( imc_mudof( arg2 ), "*" ) )
         {
            send_to_char( "* is not a valid mud name.\r\n", ch );
            return;
         }

         if( !str_cmp( imc_nameof( arg2 ), "*" ) )
         {
            send_to_char( "* is not a valid player name.\r\n", ch );
            return;
         }

         send_to_char( "Attempting to initiate IMC chess game...\r\n", ch );

         CREATE( board, GAME_BOARD_DATA, 1 );
         init_board( board );
         board->type = TYPE_IMC;
         board->player1 = QUICKLINK( ch->name );
         board->player2 = STRALLOC( arg2 );
         board->turn = -1;
         ch->pcdata->game_board = board;
         imc_send_chess( ch->name, arg2, "start" );
         return;
      }
#endif

      if( !( vch = get_char_world( ch, arg2 ) ) )
      {
         send_to_char( "Cannot find that player.\r\n", ch );
         return;
      }

      if( IS_NPC( vch ) )
      {
         send_to_char( "That player is an NPC, and cannot play games.\r\n", ch );
         return;
      }

      board = vch->pcdata->game_board;
      if( !board )
      {
         send_to_char( "That player is not playing a game.\r\n", ch );
         return;
      }

      if( board->player2 )
      {
         send_to_char( "That game already has two players.\r\n", ch );
         return;
      }

      board->player2 = QUICKLINK( ch->name );
      ch->pcdata->game_board = board;
      send_to_char( "You have joined a game of chess.\r\n", ch );

      vch = get_char_world( ch, board->player1 );
      ch_printf( vch, "%s has joined your game.\r\n", ch->name );
      return;
   }

   if( !ch->pcdata->game_board )
   {
      send_to_char( "Usage: chess <begin|cease|status|board|move|join>\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "cease" ) )
   {
      free_game( ch->pcdata->game_board );
      return;
   }

   if( !str_cmp( arg, "status" ) )
   {
      GAME_BOARD_DATA *board = ch->pcdata->game_board;

      if( !board->player1 )
         send_to_char( "There is no black player.\r\n", ch );
      else if( !str_cmp( board->player1, ch->name ) )
         send_to_char( "You are black.\r\n", ch );
      else
         ch_printf( ch, "%s is black.\r\n", board->player1 );

      if( king_in_checkmate( board, BLACK_KING ) )
         send_to_char( "The black king is in checkmate!\r\n", ch );
      else if( king_in_check( board, BLACK_KING ) )
         send_to_char( "The black king is in check.\r\n", ch );

      if( !board->player2 )
         send_to_char( "There is no white player.\r\n", ch );
      else if( !str_cmp( board->player2, ch->name ) )
         send_to_char( "You are white.\r\n", ch );
      else
         ch_printf( ch, "%s is white.\r\n", board->player2 );

      if( king_in_checkmate( board, WHITE_KING ) )
         send_to_char( "The white king is in checkmate!\r\n", ch );
      else if( king_in_check( board, WHITE_KING ) )
         send_to_char( "The white king is in check.\r\n", ch );

      if( !board->player2 || !board->player1 )
         return;

      ch_printf( ch, "%d turns.\r\n", board->turn );
      if( board->turn % 2 == 1 && !str_cmp( board->player1, ch->name ) )
      {
         ch_printf( ch, "It is %s's turn.\r\n", board->player2 );
         return;
      }
      else if( board->turn % 2 == 0 && !str_cmp( board->player2, ch->name ) )
      {
         ch_printf( ch, "It is %s's turn.\r\n", board->player1 );
         return;
      }
      else
      {
         send_to_char( "It is your turn.\r\n", ch );
         return;
      }
      return;
   }

   if( !str_prefix( arg, "board" ) )
   {
      send_to_char( print_big_board( ch, ch->pcdata->game_board ), ch );
      return;
   }

   if( !str_prefix( arg, "move" ) )
   {
      CHAR_DATA *opp;
      char opp_name[MAX_INPUT_LENGTH];
      char a, b;
      int x, y, dx, dy, ret;

      if( !ch->pcdata->game_board->player1 || !ch->pcdata->game_board->player2 )
      {
         send_to_char( "There is only 1 player.\r\n", ch );
         return;
      }

      if( ch->pcdata->game_board->turn < 0 )
      {
         send_to_char( "The game hasn't started yet.\r\n", ch );
         return;
      }

      if( king_in_checkmate( ch->pcdata->game_board, BLACK_KING ) )
      {
         send_to_char( "The black king has been checkmated, the game is over.\r\n", ch );
         return;
      }

      if( king_in_checkmate( ch->pcdata->game_board, WHITE_KING ) )
      {
         send_to_char( "The white king has been checkmated, the game is over.\r\n", ch );
         return;
      }

      if( !*argument )
      {
         send_to_char( "Usage: chess move [piece to move] [where to move]\r\n", ch );
         return;
      }

      if( ch->pcdata->game_board->turn % 2 == 1 && !str_cmp( ch->pcdata->game_board->player1, ch->name ) )
      {
         send_to_char( "It is not your turn.\r\n", ch );
         return;
      }

      if( ch->pcdata->game_board->turn % 2 == 0 && !str_cmp( ch->pcdata->game_board->player2, ch->name ) )
      {
         send_to_char( "It is not your turn.\r\n", ch );
         return;
      }

      if( sscanf( argument, "%c%d %c%d", &a, &y, &b, &dy ) != 4 )
      {
         send_to_char( "Usage: chess move [dest] [source]\r\n", ch );
         return;
      }

      if( a < 'a' || a > 'h' || b < 'a' || b > 'h' || y < 1 || y > 8 || dy < 1 || dy > 8 )
      {
         send_to_char( "Invalid move, use a-h, 1-8.\r\n", ch );
         return;
      }

      x = a - 'a';
      dx = b - 'a';
      --y;
      --dy;

      ret = is_valid_move( ch, ch->pcdata->game_board, x, y, dx, dy );
      if( ret == MOVE_OK || ret == MOVE_TAKEN )
      {
         GAME_BOARD_DATA *board;
         int piece, destpiece;

         board = ch->pcdata->game_board;
         piece = board->board[x][y];
         destpiece = board->board[dx][dy];
         board->board[dx][dy] = piece;
         board->board[x][y] = NO_PIECE;

         if( king_in_check( board, IS_WHITE( board->board[dx][dy] ) ? WHITE_KING : BLACK_KING )
             && ( board->board[dx][dy] != WHITE_KING && board->board[dx][dy] != BLACK_KING ) )
         {
            board->board[dx][dy] = destpiece;
            board->board[x][y] = piece;
            ret = MOVE_INCHECK;
         }
         else
         {
            ++board->turn;
#ifdef IMC
            if( ch->pcdata->game_board->type == TYPE_IMC )
            {
               snprintf( arg, LGST, "move %d%d %d%d", x, y, dx, dy );
               imc_send_chess( ch->pcdata->game_board->player1, ch->pcdata->game_board->player2, arg );
            }
#endif
         }
      }

      if( !str_cmp( ch->name, ch->pcdata->game_board->player1 ) )
      {
         opp = get_char_world( ch, ch->pcdata->game_board->player2 );
         if( !opp )
            mudstrlcpy( opp_name, ch->pcdata->game_board->player2, MAX_INPUT_LENGTH );
      }
      else
      {
         opp = get_char_world( ch, ch->pcdata->game_board->player1 );
         if( !opp )
            mudstrlcpy( opp_name, ch->pcdata->game_board->player1, MAX_INPUT_LENGTH );
      }

#ifdef IMC
#define SEND_TO_OPP(arg,opp) \
      if( opp ) \
      { \
         if( ch->pcdata->game_board->type == TYPE_LOCAL ) \
            ch_printf( (opp), "%s\r\n", (arg) ); \
      } \
      else \
      { \
         if( ch->pcdata->game_board->type == TYPE_IMC ) \
            imc_send_tell( ch->name, opp_name, (arg), 1 ); \
      }
#else
#define SEND_TO_OPP(arg,opp) \
      if( opp ) \
      { \
         if( ch->pcdata->game_board->type == TYPE_LOCAL ) \
            ch_printf( (opp), "%s\r\n", (arg) ); \
      }
#endif

      switch ( ret )
      {
         case MOVE_OK:
            send_to_char( "Ok.\r\n", ch );
            snprintf( arg, MAX_INPUT_LENGTH, "%s has moved.\r\n", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_INVALID:
            send_to_char( "Invalid move.\r\n", ch );
            break;

         case MOVE_BLOCKED:
            send_to_char( "You are blocked in that direction.\r\n", ch );
            break;

         case MOVE_TAKEN:
            send_to_char( "You take the enemy's piece.\r\n", ch );
            snprintf( arg, MAX_INPUT_LENGTH, "%s has taken one of your pieces!", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_CHECKMATE:
            send_to_char( "That move would result in a checkmate.\r\n", ch );
            snprintf( arg, MAX_INPUT_LENGTH, "%s has attempted a move that would result in checkmate.", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_OFFBOARD:
            send_to_char( "That move would be off the board.\r\n", ch );
            break;

         case MOVE_SAMECOLOR:
            send_to_char( "Your own piece blocks the way.\r\n", ch );
            break;

         case MOVE_CHECK:
            send_to_char( "That move would result in a check.\r\n", ch );
            snprintf( arg, MAX_INPUT_LENGTH, "%s has made a move that would result in a check.", ch->name );
            SEND_TO_OPP( arg, opp );
            break;

         case MOVE_WRONGCOLOR:
            send_to_char( "That is not your piece.\r\n", ch );
            break;

         case MOVE_INCHECK:
            send_to_char( "You are in check, you must save your king.\r\n", ch );
            break;

         default:
            bug( "%s: Unknown return value", __func__ );
            break;
      }
#undef SEND_TO_OPP
      return;
   }
   send_to_char( "Usage: chess <begin|cease|status|board|move|join>\r\n", ch );
}
