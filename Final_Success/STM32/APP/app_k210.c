#include "app_k210.h"

char buf_msg[20] = {'\0'};
uint8_t g_new_flag = 0;
uint8_t g_index = 0;
uint8_t g_new_data = 0;

void Deal_K210(uint8_t recv_msg)
{
	if (recv_msg == '$' && g_new_flag == 0)
	{
		g_new_flag = 1;
		memset(buf_msg, 0, sizeof(buf_msg));
		return;
	}
	if(g_new_flag == 1)
	{
		if (recv_msg == '#')
		{
			g_new_flag = 0;
			g_index = 0;
			g_new_data = 1;
		}

		if (g_new_flag == 1 && recv_msg != '$')
		{
			buf_msg[g_index++] = recv_msg;

			if(g_index > 20)
			{
				g_index = 0;
				g_new_flag = 0;
				g_new_data = 0;
				memset(buf_msg, 0, sizeof(buf_msg));
			}

		}
	}
}

#define Trun_speed 250
#define Go_speed 8

void Change_state(void)
{
	if(g_new_data == 1)
	{
		g_new_data = 0;
		if (strcmp("goback", buf_msg) == 0 )
		{
			Move_X = -Go_speed;
			my_delay(2);
			Move_X = 0;
		}
		else if (strcmp("goahead", buf_msg) == 0 )
		{
			Move_X = Go_speed;
			my_delay(2);
			Move_X = 0;
		}
		else if (strcmp("turnleft", buf_msg) == 0)
		{
			Move_Z = -Trun_speed;
			my_delay(1);

			Move_Z = 0;
			Move_X = Go_speed;

			my_delay(1);
			Move_X = 0;
		}
		else if (strcmp("turnright", buf_msg) == 0 )
		{
			Move_Z = Trun_speed;
			my_delay(1);

			Move_Z = 0;
			Move_X = Go_speed;

			my_delay(1);
			Move_X = 0;
		}

	}

}
