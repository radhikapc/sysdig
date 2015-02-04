/*
Copyright (C) 2013-2014 Draios inc.

This file is part of sysdig.

sysdig is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

sysdig is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sysdig.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string>
#include <unordered_map>
#include <map>
#include <queue>
#include <vector>
#include <set>
using namespace std;

#include "sinsp.h"
#include "sinsp_int.h"
#include "../../driver/ppm_ringbuffer.h"
#include "filter.h"
#include "filterchecks.h"

#ifdef SYSTOP

#include <curses.h>
#include "table.h"
#include "cursestable.h"

curses_table::curses_table()
{
	m_selct = 0;
	m_firstrow = 0;
	m_data = NULL;
	m_table = NULL;
	m_table_y_start = TABLE_Y_START;

	m_converter = new sinsp_filter_check_reference();

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			init_pair((7-i)*8+j, i, (j==0?-1:j));
		}
	}

	//
	// Colors initialization
	//
	m_colors[RESET_COLOR] = ColorPair( COLOR_WHITE,COLOR_BLACK);
	m_colors[DEFAULT_COLOR] = ColorPair( COLOR_WHITE,COLOR_BLACK);
	m_colors[FUNCTION_BAR] = ColorPair(COLOR_BLACK,COLOR_CYAN);
	m_colors[FUNCTION_KEY] = ColorPair( COLOR_WHITE,COLOR_BLACK);
	m_colors[PANEL_HEADER_FOCUS] = ColorPair(COLOR_BLACK,COLOR_GREEN);
	m_colors[PANEL_HEADER_UNFOCUS] = ColorPair(COLOR_BLACK,COLOR_GREEN);
	m_colors[PANEL_HIGHLIGHT_FOCUS] = ColorPair(COLOR_BLACK,COLOR_CYAN);
	m_colors[PANEL_HIGHLIGHT_UNFOCUS] = ColorPair(COLOR_BLACK, COLOR_WHITE);
	m_colors[FAILED_SEARCH] = ColorPair(COLOR_RED,COLOR_CYAN);
	m_colors[UPTIME] = A_BOLD | ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[BATTERY] = A_BOLD | ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[LARGE_NUMBER] = A_BOLD | ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[METER_TEXT] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[METER_VALUE] = A_BOLD | ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[LED_COLOR] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[TASKS_RUNNING] = A_BOLD | ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[PROCESS] = A_NORMAL;
	m_colors[PROCESS_SHADOW] = A_BOLD | ColorPair(COLOR_BLACK,COLOR_BLACK);
	m_colors[PROCESS_TAG] = A_BOLD | ColorPair(COLOR_YELLOW,COLOR_BLACK);
	m_colors[PROCESS_MEGABYTES] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[PROCESS_BASENAME] = A_BOLD | ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[PROCESS_TREE] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[PROCESS_R_STATE] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[PROCESS_D_STATE] = A_BOLD | ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[PROCESS_HIGH_PRIORITY] = ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[PROCESS_LOW_PRIORITY] = ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[PROCESS_THREAD] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[PROCESS_THREAD_BASENAME] = A_BOLD | ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[BAR_BORDER] = A_BOLD;
	m_colors[BAR_SHADOW] = A_BOLD | ColorPair(COLOR_BLACK,COLOR_BLACK);
	m_colors[SWAP] = ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[GRAPH_1] = A_BOLD | ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[GRAPH_2] = ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[GRAPH_3] = A_BOLD | ColorPair(COLOR_YELLOW,COLOR_BLACK);
	m_colors[GRAPH_4] = A_BOLD | ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[GRAPH_5] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[GRAPH_6] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[GRAPH_7] = A_BOLD | ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[GRAPH_8] = ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[GRAPH_9] = A_BOLD | ColorPair(COLOR_BLACK,COLOR_BLACK);
	m_colors[MEMORY_USED] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[MEMORY_BUFFERS] = ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[MEMORY_BUFFERS_TEXT] = A_BOLD | ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[MEMORY_CACHE] = ColorPair(COLOR_YELLOW,COLOR_BLACK);
	m_colors[LOAD_AVERAGE_FIFTEEN] = A_BOLD | ColorPair(COLOR_BLACK,COLOR_BLACK);
	m_colors[LOAD_AVERAGE_FIVE] = A_NORMAL;
	m_colors[LOAD_AVERAGE_ONE] = A_BOLD;
	m_colors[LOAD] = A_BOLD;
	m_colors[HELP_BOLD] = A_BOLD | ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[CLOCK] = A_BOLD;
	m_colors[CHECK_BOX] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[CHECK_MARK] = A_BOLD;
	m_colors[CHECK_TEXT] = A_NORMAL;
	m_colors[HOSTNAME] = A_BOLD;
	m_colors[CPU_NICE] = ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[CPU_NICE_TEXT] = A_BOLD | ColorPair(COLOR_BLUE,COLOR_BLACK);
	m_colors[CPU_NORMAL] = ColorPair(COLOR_GREEN,COLOR_BLACK);
	m_colors[CPU_KERNEL] = ColorPair(COLOR_RED,COLOR_BLACK);
	m_colors[CPU_IOWAIT] = A_BOLD | ColorPair(COLOR_BLACK, COLOR_BLACK);
	m_colors[CPU_IRQ] = ColorPair(COLOR_YELLOW,COLOR_BLACK);
	m_colors[CPU_SOFTIRQ] = ColorPair(COLOR_MAGENTA,COLOR_BLACK);
	m_colors[CPU_STEAL] = ColorPair(COLOR_CYAN,COLOR_BLACK);
	m_colors[CPU_GUEST] = ColorPair(COLOR_CYAN,COLOR_BLACK);

	//
	// Column sizes initialization
	//
	m_colsizes[PT_NONE] = 0;
	m_colsizes[PT_INT8] = 8;
	m_colsizes[PT_INT16] = 8;
	m_colsizes[PT_INT32] = 8;
	m_colsizes[PT_INT64] = 8;
	m_colsizes[PT_UINT8] = 8;
	m_colsizes[PT_UINT16] = 8;
	m_colsizes[PT_UINT32] = 8;
	m_colsizes[PT_UINT64] = 8;
	m_colsizes[PT_CHARBUF] = 32;
	m_colsizes[PT_BYTEBUF] = 32;
	m_colsizes[PT_ERRNO] = 8;
	m_colsizes[PT_SOCKADDR] = 16;
	m_colsizes[PT_SOCKTUPLE] = 16;
	m_colsizes[PT_FD] = 32;
	m_colsizes[PT_PID] = 16;
	m_colsizes[PT_FDLIST] = 16;
	m_colsizes[PT_FSPATH] = 32;
	m_colsizes[PT_SYSCALLID] = 8;
	m_colsizes[PT_SIGTYPE] = 8;
	m_colsizes[PT_RELTIME] = 16;
	m_colsizes[PT_ABSTIME] = 16;
	m_colsizes[PT_PORT] = 8;
	m_colsizes[PT_L4PROTO] = 8;
	m_colsizes[PT_SOCKFAMILY] = 8;
	m_colsizes[PT_BOOL] = 8;
	m_colsizes[PT_IPV4ADDR] = 8;
	m_colsizes[PT_DYN] = 8;
	m_colsizes[PT_FLAGS8] = 32;
	m_colsizes[PT_FLAGS16] = 32;
	m_colsizes[PT_FLAGS32] = 32;
	m_colsizes[PT_UID] = 12;
	m_colsizes[PT_GID] = 12;
	m_colsizes[PT_MAX] = 0;

	//
	// Define the table size
	//
	getmaxyx(stdscr, m_screenh, m_screenw);
	m_w = TABLE_WIDTH;
	//m_h = TABLE_HEIGHT;
	m_h = m_screenh - 2;
	m_scrolloff_x = 0;
	m_scrolloff_y = 10;

	//
	// Create the window
	//
	refresh();
	m_win = newwin(m_h, 500, m_table_y_start, 0);

	//
	// Pipulate the main menu entries
	//
	m_menuitems.push_back("Help");
	m_menuitems.push_back("View");
	m_menuitems.push_back("Setup");
	m_menuitems.push_back("Search");
}

curses_table::~curses_table()
{
	delwin(m_win);
	delete m_converter;
}

void curses_table::configure(sinsp_table* table, vector<int32_t>* colsizes)
{
	uint32_t j;

	m_table = table;
	vector<filtercheck_field_info>* legend = m_table->get_legend();

	if(colsizes)
	{
		if(colsizes->size() != legend->size())
		{
			throw sinsp_exception("invalid table legend: column size doesn't match");
		}
	}

	for(j = 1; j < legend->size(); j++)
	{
		curses_table_column_info ci;
		ci.m_info = legend->at(j);

		if(colsizes == NULL || colsizes->at(j) == -1)
		{
			ci.m_size = m_colsizes[legend->at(j).m_type];
		}
		else
		{
			ci.m_size = colsizes->at(j);		
		}

		int32_t namelen = strlen(ci.m_info.m_name);
		if(ci.m_size < namelen + 1)
		{
			ci.m_size = namelen + 1;
		}

		m_legend.push_back(ci);
	}
}

void curses_table::update_rowkey(int32_t row)
{
	sinsp_table_field* rowkey = m_table->get_row_key(row);

	if(rowkey != NULL)
	{
		m_last_key.copy(rowkey);
		m_last_key.m_isvalid = true;
	}
	else
	{
		m_last_key.m_isvalid = false;
	}
}

void curses_table::update_data(vector<sinsp_sample_row>* data)
{
	m_data = data;

	if(!m_last_key.m_isvalid)
	{
		update_rowkey(m_selct);
	}
	else
	{
		m_selct = m_table->get_row_from_key(&m_last_key);
		if(m_selct == -1)
		{
			m_selct = 0;
			m_firstrow = 0;
			m_last_key.m_isvalid = false;
		}
		else
		{
			selection_goto(m_selct);			
		}

		sanitize_selection();
	}
}

void curses_table::render_main_menu()
{
	uint32_t j = 0;
	uint32_t k = 0;

	for(j = 0; j < m_menuitems.size(); j++)
	{
		attrset(m_colors[PROCESS]);
		string fks = string("F") + to_string(j + 1);
		mvaddnstr(m_screenh - 1, k, fks.c_str(), 2);
		k += 2;

		attrset(m_colors[PANEL_HIGHLIGHT_FOCUS]);
		fks = m_menuitems[j];
		fks.resize(6, ' ');
		mvaddnstr(m_screenh - 1, k, fks.c_str(), 6);
		k += 6;
	}
}

void curses_table::render(bool data_changed)
{
	uint32_t j, k;
	int32_t l, m;
	char bgch = ' ';

	if(m_data == NULL)
	{
		return;
	}

	if(m_data->size() != 0)
	{
		if(m_legend.size() != m_data->at(0).m_values.size())
		{
			ASSERT(false);
			throw sinsp_exception("corrupted curses table data");
		}
	}

	if(data_changed)
	{
		m_column_startx.clear();

		if(m_selct < 0)
		{
			m_selct = 0;
		}
		else if(m_selct > (int32_t)m_data->size() - 1)
		{
			m_selct = (int32_t)m_data->size() - 1;
		}

		wattrset(m_win, m_colors[PANEL_HEADER_FOCUS]);

		//
		// Render the column headers
		//
		wmove(m_win, 0, 0);
		for(j = 0; j < m_w; j++)
		{
			waddch(m_win, ' ');
		}

		for(j = 0, k = 0; j < m_legend.size(); j++)
		{
			if(j == m_table->get_sorting_col())
			{
				wattrset(m_win, m_colors[PANEL_HIGHLIGHT_FOCUS]);
			}
			else
			{
				wattrset(m_win, m_colors[PANEL_HEADER_FOCUS]);
			}

			m_column_startx.push_back(k);
			mvwaddnstr(m_win, 0, k, m_legend[j].m_info.m_name, m_legend[j].m_size);

			for(l = strlen(m_legend[j].m_info.m_name); l < m_legend[j].m_size; l++)
			{
				waddch(m_win, ' ');
			}

			k += m_legend[j].m_size;
		}

		//
		// Render the rows
		//
		vector<sinsp_table_field>* row;

		for(l = 0; l < (int32_t)MIN(m_data->size(), m_h - 1); l++)
		{
			if(l + m_firstrow >= (int32_t)m_data->size())
			{
				break;
			}

			row = &(m_data->at(l + m_firstrow).m_values);

			if(l == m_selct - (int32_t)m_firstrow)
			{
				wattrset(m_win, m_colors[PANEL_HIGHLIGHT_FOCUS]);
			}
			else
			{
				wattrset(m_win, m_colors[PROCESS]);
			}

			//
			// Render the rows
			//
			wmove(m_win, l + 1, 0);
			for(j = 0; j < m_w; j++)
			{
				waddch(m_win, bgch);
			}

			for(j = 0, k = 0; j < m_legend.size(); j++)
			{
				m_converter->set_val(m_legend[j].m_info.m_type, row->at(j).m_val, row->at(j).m_len);
				mvwaddnstr(m_win, l + 1, k, m_converter->tostring_nice(NULL), m_legend[j].m_size);
				k += m_legend[j].m_size;
			}
		}

		wattrset(m_win, m_colors[PROCESS]);

		if(l < (int32_t)m_h - 1)
		{
			for(m = l; m < (int32_t)m_h - 1; m++)
			{
				wmove(m_win, m + 1, 0);

				for(j = 0; j < m_w; j++)
				{
					waddch(m_win, ' ');
				}
			}
		}
	}

	wrefresh(m_win);
	copywin(m_win,
		stdscr,
		0,
		m_scrolloff_x,
		m_scrolloff_y,
		0,
		m_scrolloff_y + (m_h - 1),
		m_screenw - 1,
		FALSE);

	wrefresh(m_win);

	//
	// Draw the menu at the bottom of the screen
	//
	render_main_menu();

	refresh();
}

void curses_table::scrollwin(uint32_t x, uint32_t y)
{
	wrefresh(m_win);

	m_scrolloff_x = x;
	m_scrolloff_y = y;

	render(false);
}

void curses_table::sanitize_selection()
{
	if(m_firstrow > (int32_t)(m_data->size() - m_h + 1))
	{
		m_firstrow = m_data->size() - m_h + 1;
	}
	
	if(m_firstrow < 0)
	{
		m_firstrow = 0;
	}	

	if(m_selct > (int32_t)m_data->size() - 1)
	{
		m_selct = m_data->size() - 1;
	}
	
	if(m_selct < 0)
	{
		m_selct = 0;
	}	

	if(m_firstrow > m_selct)
	{
		m_firstrow = m_selct;
	}
}

void curses_table::selection_up()
{
	if(m_selct > 0)
	{
		if(m_selct <= (int32_t)m_firstrow)
		{
			m_firstrow--;
		}

		m_selct--;
		sanitize_selection();
		update_rowkey(m_selct);
		render(true);
	}
}

void curses_table::selection_down()
{
	if(m_selct < (int32_t)m_data->size() - 1)
	{
		if(m_selct - m_firstrow > (int32_t)m_h - 3)
		{
			m_firstrow++;
		}

		m_selct++;
		sanitize_selection();
		update_rowkey(m_selct);
		render(true);
	}
}

void curses_table::selection_pageup()
{
	m_firstrow -= (m_h - 1);
	m_selct -= (m_h - 1);

	sanitize_selection();
	update_rowkey(m_selct);
	render(true);
}

void curses_table::selection_pagedown()
{
	m_firstrow += (m_h - 1);
	m_selct += (m_h - 1);

	sanitize_selection();
	update_rowkey(m_selct);
	render(true);
}

void curses_table::selection_goto(int32_t row)
{
	if(row == -1 ||
		row >= (int32_t)m_data->size())
	{
		ASSERT(false);
		return;
	}

	m_firstrow = row - (m_h /2);
	m_selct = row;

	sanitize_selection();
	render(true);
}


//
// Return false if the user wants us to exit
//
bool curses_table::handle_input(int ch)
{
	switch(ch)
	{
		case 'q':
			return false;
/*
		case 'a':
			numbers[0]++;
			render(true);
			break;
		case KEY_LEFT:
			if(scrollpos > 0)
			{
				scrollpos--;
				scrollwin(scrollpos, 10);
			}
			break;
		case KEY_RIGHT:
			if(scrollpos < TABLE_WIDTH - (int32_t)m_screenw)
			{
				scrollpos++;
				scrollwin(scrollpos, 10);
			}
			break;
*/			
		case KEY_UP:
			selection_up();
			break;
		case KEY_DOWN:
			selection_down();
			break;
		case KEY_PPAGE:
			selection_pageup();
			break;
		case KEY_NPAGE:
			selection_pagedown();
			break;
		case KEY_F(1):
			mvprintw(0, 0, "F1");
			refresh();
			break;
		case KEY_F(2):
			mvprintw(0, 0, "F1");
			refresh();
			break;
		case KEY_MOUSE:
			{
				uint32_t j;
				MEVENT event;

				if(getmouse(&event) == OK)
				{
//					if(event.bstate & BUTTON1_PRESSED)
					{
						ASSERT((m_data->size() == 0) || (m_column_startx.size() == m_data->at(0).m_values.size()));

						if((uint32_t)event.y == m_table_y_start)
						{
							//
							// This is a click on a column header. Change the sorting accordingly.
							//
							for(j = 0; j < m_column_startx.size() - 1; j++)
							{
								if((uint32_t)event.x >= m_column_startx[j] && (uint32_t)event.x < m_column_startx[j + 1])
								{
									m_table->set_sorting_col(j + 1);
									break;
								}
							}

							if(j == m_column_startx.size() - 1)
							{
								m_table->set_sorting_col(j + 1);
							}

							render(true);
						}
						else if((uint32_t)event.y > m_table_y_start &&
							(uint32_t)event.y < m_table_y_start + m_h - 1)
						{
							//
							// This is a click on a row. Update the selection.
							//
							m_selct = event.y - m_table_y_start - 1;
							sanitize_selection();
							update_rowkey(m_selct);
							render(true);
						}
					}
				}
			}
			break;
	}

	return true;
}

#endif // SYSTOP
