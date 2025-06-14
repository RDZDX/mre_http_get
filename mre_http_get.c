#include "mre_http_get.h"

VMUINT8* screenbuf = NULL;	
VMINT ncharacter_height = -1;
VMINT nscreen_width = -1;
VMINT nscreen_height = -1;

//#define DOWNLOAD_URL "http://sneaindia.com/four.jpeg"
//#define DOWNLOAD_URL "http://www.photo-dictionary.com/img/picture-dictionary.png"
#define CHUNK_SIZE 200

VMWCHAR title_name[22] = {0};
VMBOOL first = VM_FALSE;
VMBOOL second = VM_FALSE;
VMBOOL third = VM_FALSE;
VMCHAR new_data[1201] = {0};

static VMUINT file_size = 0;
static VMCHAR file_name[128] = {0};
static VMFILE file_hdl = -1;
static VMINT http_hdl[12] = {-1};
VMWSTR ucs2_buf = 0;
VMCHAR*	app_text = NULL;

int filledDsplByLines = 0;

VMBOOL flightMode = VM_FALSE;	

VMCHAR tmp_filename[128];
VMCHAR tmp_prefix[128];
VMINT drv;

static VMCHAR file_name1[128] = {0};

VMCHAR download_fname[128];
VMCHAR n_without_ext[128];
VMCHAR extension_name[5];

void vm_main(void) {

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
        vm_font_set_font_size(VM_SMALL_FONT);
        ncharacter_height = vm_graphic_get_character_height();
        nscreen_width = vm_graphic_get_screen_width();
        nscreen_height = vm_graphic_get_screen_height();
        vm_ascii_to_ucs2(title_name, (strlen("Input URL:") + 1) * 2, "Input URL:");
        vm_input_set_editor_title(title_name);
        if (vm_sim_card_count() == 99) { flightMode = VM_TRUE; }
        drv = vm_get_removable_driver();
        if (drv < 0) {drv = vm_get_system_driver();}
        if (flightMode == VM_FALSE) {vm_input_text3(NULL, 1200, 8, save_text);}
}

void handle_sysevt(VMINT message, VMINT param) {

	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		layer_hdl[0] = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1);	
		vm_graphic_set_clip(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
                fill_white();
		break;

	case VM_MSG_PAINT:
                vm_switch_power_saving_mode(turn_off_mode);
	        screenbuf = vm_graphic_get_layer_buffer(layer_hdl[0]);
                if (second == VM_TRUE) {
                   vm_exit_app();
                } else if (flightMode == VM_TRUE) {
                   display_text_line(screenbuf, "Please turn Flight mode off !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
                } else {
                }
		break;
		
	case VM_MSG_INACTIVE:
                vm_switch_power_saving_mode(turn_on_mode);
		if( layer_hdl[0] != -1 )
			vm_graphic_delete_layer(layer_hdl[0]);
		
		break;	
	case VM_MSG_QUIT:
		if( layer_hdl[0] != -1 )
			vm_graphic_delete_layer(layer_hdl[0]);
		
		break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_RIGHT_SOFTKEY) {
        if (layer_hdl[0] != -1) {
            vm_graphic_delete_layer(layer_hdl[0]);
            layer_hdl[0] = -1;
        }
        vm_exit_app();
    }

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_LEFT_SOFTKEY) {
        break_download(get_con_handl());
        //display_text_line(screenbuf, DOWNLOAD_URL, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
        //display_text_line(screenbuf, "Connecting...", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
        //let_download();

    }

}

void save_text(VMINT state, VMWSTR text) {

    VMINT lenght;

    lenght = wstrlen(text);

    if (state == VM_INPUT_OK && lenght > 0) {
       vm_ucs2_to_ascii(new_data, lenght + 1, text);
       display_text_line(screenbuf, new_data, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
       display_text_line(screenbuf, "Connecting...", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
       let_download();
    } else {
       //strcpy(new_data , "");
       second = VM_TRUE;
       vm_exit_app();
       return;
    }


}

static void fill_white(void) {

	vm_graphic_color color;

	color.vm_color_565 = VM_COLOR_WHITE;
	vm_graphic_setcolor(&color);
	vm_graphic_fill_rect_ex(layer_hdl[0], 0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
	vm_graphic_flush_layer(layer_hdl, 1);
        filledDsplByLines = 0;

}

VMINT string_width(VMWCHAR *whead, VMWCHAR *wtail) {

	VMWCHAR * wtemp = NULL;
	VMINT width = 0;
	if (whead == NULL || wtail == NULL)
	return 0;
	wtemp = (VMWCHAR *)vm_malloc((wtail - whead) * 2 + 2);
	
	if (wtemp == NULL)
	return 0;
	memset(wtemp, 0, (wtail - whead) * 2 + 2);
	memcpy(wtemp, whead, (wtail - whead) * 2);

	width = vm_graphic_get_string_width(wtemp);
	vm_free(wtemp);
	return width;
}

void display_text_line(VMUINT8 *disp_buf, VMSTR str, VMINT x, VMINT y, VMINT width, VMINT height, VMINT betlines, VMINT startLine, VMINT color, VMBOOL fix_pos) {

	VMWCHAR *ucstr;
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;
	VMINT nheight = y; 
	VMINT nline_height ;
	VMINT nlines = 0;

        if (y = 0) {fill_white();}

	if (str == NULL||disp_buf==NULL||betlines < 0) {return;}

	nline_height = vm_graphic_get_character_height() + betlines;

        if (third == VM_TRUE && fix_pos == VM_FALSE){
           nheight = nheight + nline_height;
           third = VM_FALSE;
        }

	if (fix_pos == VM_TRUE) {
	   vm_graphic_fill_rect(screenbuf, 0, filledDsplByLines, nscreen_width, nline_height, VM_COLOR_WHITE, VM_COLOR_WHITE);
           third = VM_TRUE;
	}

	ucstr = (VMWCHAR*)vm_malloc(2*(strlen(str)+1));

	if (ucstr == NULL) {return;}
	
	if(0 != vm_ascii_to_ucs2(ucstr,2*(strlen(str)+1),str)) {
		vm_free(ucstr);
		return ;
	}
	ucshead = ucstr;
	ucstail = ucshead + 1;
	
	while(is_end == FALSE)
	{
		//if (nheight+nline_height>y+height) // paliekam paskutine ekrano eilute del meniu
		if (nheight > y + height) { // jeigu uzimtas ekrano aukstis virsijo 320
                        fill_white();
                        nheight = 0;
			//break;
                }

		while (1)
		{
			if (string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				nlines++;
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				nlines++;
				break;
			}
		}
		if ( nlines >= startLine)
		{
			vm_graphic_textout(disp_buf, x, nheight, ucshead, (ucstail-ucshead), (VMUINT16)(color));
                        vm_graphic_flush_layer(layer_hdl, 1);
			if (fix_pos == VM_FALSE) {nheight += nline_height;}
                        filledDsplByLines = nheight;
		}
		ucshead = ucstail;
		ucstail ++;
	}
	vm_free(ucstr);
}

void extract_end_text(char *result_data, const char *inp_data, char separator) {
    int i = 0;
    int last_sep_pos = -1;

    while (inp_data[i] != '\0') {
        if (inp_data[i] == separator) {
            last_sep_pos = i;
        }
        i++;
    }

    // Copy characters after the last separator
    i = 0;
    int j = last_sep_pos + 1;
    while (inp_data[j] != '\0') {
        result_data[i++] = inp_data[j++];
    }
    result_data[i] = '\0';  // Null-terminate the string
}

void let_download(void) {

    //download_process(DOWNLOAD_URL, "", HTTP_USE_CMNET_PRIORITY, GET, &http_hdl[0], download_callback, notify_callback);
    download_process(new_data, "", HTTP_USE_CMNET_PRIORITY, GET, &http_hdl[0], download_callback, notify_callback);

}

void break_download(VMINT handle) {

    VMCHAR text[255] = {0};
    if (!vm_cancel_asyn_http_req(handle)) {
        handle = -1;
        sprintf(text, "Interrupt HTTP download");
        display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
    }
}

static VMINT download_process(const VMCHAR* url, const VMCHAR* cnt,
                          VM_HTTP_PROXY_TYPE apn, HTTP_METHOD mth,
                          VMINT* handle,
                          void (*download_callback)(VMINT bResponse, void* pSession),
                          void (*notify_callback)(VMINT state, VMINT param,
                                                       void* session)) {

    asyn_http_req_t req;
    VMINT ret;
    http_head_t head[1];
    VMCHAR full_name[128];
    VMINT find_hdl;
    VMUINT start_size;
    struct vm_fileinfo_t info;

    VMCHAR text[255] = {0};

    VMWCHAR w_full_name[258];

    extract_end_text(download_fname, url, '/');           // is URL istraukiam siunciamo failo varda "picture.jpg" 
    if (strlen(url) == strlen(download_fname)) {
       display_text_line(screenbuf, "Wrong URL !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
       return;
    }
    extract_end_text(extension_name, download_fname, '.'); // is siunciamo failo vardo istraukiame prapletima "jpg"
    if (strlen(extension_name) == 0) {
       display_text_line(screenbuf, "Wrong URL !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
       return;
    }
    strncpy(n_without_ext, download_fname, strlen(download_fname) - strlen(extension_name));
    n_without_ext[strlen(download_fname) - strlen(extension_name)] = '\0';

    sprintf(full_name, "%c:\\%s*", drv, n_without_ext);

    vm_ascii_to_ucs2(w_full_name, (strlen(full_name) + 1) * 2, full_name);

    if ((find_hdl = vm_find_first(w_full_name, &info)) >= 0) {
        vm_ucs2_to_gb2312(file_name, 128, info.filename); //50
        if (!vm_ends_with(file_name, extension_name)) {
            sprintf(text, "The file has been downloaded");
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            return -1;
        } else if (!vm_ends_with(file_name, "tmp")) {

            sprintf(full_name, "%c:\\%s", drv, file_name);
            vm_ascii_to_ucs2(w_full_name, (strlen(full_name) + 1) * 2, full_name);
            read_data(w_full_name, 0, 4, &file_size);
            read_data(w_full_name, 4, 4, &start_size);
        }
        vm_find_close(find_hdl);
    }

    /*Request method*/
    req.req_method = mth;
    /*Network access method*/
    req.use_proxy = apn;

    /*Assembling HTTP requests*/
    req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t));
    if (NULL == req.http_request) {
        return -1;
    }

    /*Assembling the URL*/
    if (strncmp(url, "http://", strlen("http://"))) {
        strcat(req.http_request->url, "http://");
    }
    strcat(req.http_request->url, url);

    /*Assembling HTTP request headers*/
    if (0 == file_size) {
        sprintf(head[0].name, "RANGE");
        sprintf(head[0].value, "bytes=-1");

        memset(text, 0, 255);
        //sprintf(text, "Request to obtain %s size", download_fname);
        //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
    } else {
        sprintf(head[0].name, "RANGE");
        memset(text, 0, 255);
        sprintf(head[0].value, "bytes=%d-%d", start_size, ((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
        //sprintf(text, "Request to download file: from %d byte to %d byte", start_size, ((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
        //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
    }
    req.http_request->nhead = 1;
    req.http_request->heads = head;

    /*Sending HTTP Request*/
    ret = vm_asyn_http_req(&req, download_callback, notify_callback);

    /*Get HTTP handle*/
    if (ASYN_HTTP_REQ_ACCEPT_SUCCESS == ret) {
        ret = vm_get_asyn_http_req_handle(&req, handle);
    }

    vm_free(req.http_request);

    return ret;
}

static void download_callback(VMINT bResponse, void* pSession) {

    VMCHAR head[255] = {0};
    VMCHAR text[255] = {0};

    VMUINT written;
    VMUINT start_size = 0;

    http_session_t* session = NULL;

    VMWCHAR w_file_name[258] = {0};
    VMWCHAR w_file_name1[258] = {0};

    sprintf(file_name, "%c:\\%stmp", drv, n_without_ext); //dublicate?
    //display_text_line(screenbuf, file_name, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);

    /*Correct Response*/
    if (0 == bResponse) {
        session = (http_session_t*)pSession;

        /*Server Error*/
        if (0 > session->res_code || 500 == session->res_code) {
            strcpy(text, "Server Error");
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            return;
        }

        /*Get Content-Type*/
        if (get_http_head(session, "Content-Type", head)) {
            strcpy(text, "Failed to obtain Content-Type");
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            return;
        }

        /*The one that starts with "text/vnd.wap.wml" indicates that the
         * connection is CMWAP and needs to be reconnected.*/
        if (!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) {
            strcpy(text, "CMWAP Reconnect");
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            let_download();
            return;
        }

        /*The full path name of the temporary file generated by downloading*/
        sprintf(file_name, "%c:\\%stmp", drv, n_without_ext); //dublicate?
        //display_text_line(screenbuf, file_name, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);

        vm_ascii_to_ucs2(w_file_name, (strlen(file_name) + 1) * 2, file_name);

        /*Download the file for the first time*/
        if (0 == file_size) {
            /*Creating a temporary file*/
            file_hdl = vm_file_open(w_file_name, MODE_CREATE_ALWAYS_WRITE, TRUE);
            if (file_hdl >= 0) {
                /*Get Content-Range*/
                if (get_http_head(session, "Content-Range", head)) {
                    vm_file_close(file_hdl);
                    vm_file_delete(w_file_name);
                    strcpy(text, "Failed to get file size, tmpfile deleted");
                    display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
                    return;
                }

                /*Get file size*/
                //file_size = strtoi(strstr(head, "/") + 1);

                char* slash_ptr = strstr(head, "/");
                if (slash_ptr && isdigit(*(slash_ptr + 1))) {
                    file_size = strtoi(slash_ptr + 1);
                } else {
                    strcpy(text, "Invalid Content-Range");
                    display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
                    return;
                }


                /*The first 4 bytes of the temporary file store the file size*/
                vm_file_write(file_hdl, &file_size, 4, &written);
                /*The next 4 bytes store the location where the next download
                 * will start*/
                vm_file_write(file_hdl, &start_size, 4, &written);
                vm_file_close(file_hdl);
            }

            memset(text, 0, 255);
            //sprintf(text, "File size: %d bytes", file_size);
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
        }
        /*This is not the first time you download the file*/
        else {
            /*Update the location where the next download starts*/
            read_data(w_file_name, 4, 4, &start_size);
            start_size += session->nresbody;

            
            int progress = (int)(((float)start_size / file_size) * 100); // Apskaičiuojame progresą
            // Rodyti progresą
            memset(text, 0, sizeof(text));
            sprintf(text, "Download progress: %d%%", progress);
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_TRUE);

            file_hdl = vm_file_open(w_file_name, MODE_WRITE, TRUE);
            vm_file_seek(file_hdl, 4, BASE_BEGIN);
            vm_file_write(file_hdl, &start_size, 4, &written);

            /*Write the downloaded data to a temporary file*/
            vm_file_seek(file_hdl, 0, BASE_END);
            vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
            vm_file_close(file_hdl);

            memset(text, 0, 255);
            //sprintf(text, "Download successful, a total of %d bytes downloaded", session->nresbody);
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);

            /*Download Complete*/
            if (start_size == file_size) {
                VMCHAR* data;
                data = vm_malloc(file_size);
                read_data(w_file_name, 8, file_size, data);
                vm_file_delete(w_file_name);
                /*Save as new file*/
                sprintf(file_name, "%c:\\%s", drv, download_fname);
                vm_ascii_to_ucs2(w_file_name1, (strlen(file_name) + 1) * 2, file_name);
                file_hdl = vm_file_open(w_file_name1, MODE_CREATE_ALWAYS_WRITE, TRUE);
                vm_file_write(file_hdl, data, file_size, &written);
                vm_file_close(file_hdl);
                vm_free(data);
                data = NULL;
                http_hdl[0] = -1;
                sprintf(text, "Download successful, a total of %d bytes downloaded", file_size);
                display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            }
        }
    }

    /*Continue downloading the rest of the file*/
    let_download();
}

static void notify_callback(VMINT state, VMINT param, void* session) {

    VMCHAR text[255] = "";

    switch (state) {
        case HTTP_STATE_GET_HOSTNAME:
            //strcpy(text, "Getting Host");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        case HTTP_STATE_CONNECTING:
            //strcpy(text, "Connecting");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        case HTTP_STATE_SENDING:
            //strcpy(text, "Sending request message");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        case HTTP_STATE_RECV_STATUS:
            //strcpy(text, "Receiving status information");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        case HTTP_STATE_RECV_HEADS:
            //strcpy(text, "Receiving response headers");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        case HTTP_STATE_RECV_BODY:
            //strcpy(text, "Receiving response body");
            //display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
        default:
            strcpy(text, "Unknown Event");
            display_text_line(screenbuf, text, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK, VM_FALSE);
            break;
    }

}

VMINT read_data(VMWSTR file_name, VMINT offset, VMINT num, void* data)
{
	VMFILE file_hdl;
	VMUINT nread;
	file_hdl = vm_file_open(file_name, MODE_READ, TRUE);
	if(file_hdl >= 0)
	{		
		vm_file_seek(file_hdl, offset, BASE_BEGIN);
		vm_file_read(file_hdl, data, num, &nread);
		vm_file_close(file_hdl);
		return 0;
	}
	return -1;
}

void break_all(void)
{
	vm_cancel_all_http_sessions();
        http_hdl[0] = -1;
}

VMINT get_con_handl(void) {

	return http_hdl[0];
}