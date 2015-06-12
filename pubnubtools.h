
#include "pubnub.h"
#include "pubnub.hpp"
#include "pubnub-sync.h"
#include "pubnub-sync.hpp"
#include "pubnub-priv.h"
#include "pubnub-libevent.h"

#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"


//#include "../libpubnub-cpp/pubnub.cpp"
//#include "../libpubnub-cpp/pubnub-sync.cpp"

#undef pubnub_init
#undef pubnub_done
#undef pubnub_publish
#undef pubnub_subscribe
#undef pubnub_subscribe_multi
#undef pubnub_history
#undef pubnub_here_now
#undef pubnub_time

class playlist_callback_manager : public play_callback
{
public:

	pfc::string8 tr = "";
	bool ret;
	std::string pubkey = "";
	std::string subkey = "";
	std::string origin = "";
	std::string channel = "";

	/*
	virtual unsigned get_flags() {
	return play_callback::flag_on_playback_all;
	}
	*/

	void pubNubPublish(struct json_object* msg)
	{
		pubnub_sync *sync = pubnub_sync_init();

		PubNub pn(pubkey, subkey, &pubnub_sync_callbacks, sync);
		pn.set_origin(origin);
		pn.publish(channel, *msg, -1, NULL);
	}

	void setKeys(std::string pub, std::string sub, std::string orig, std::string ch)
	{
		pubkey = pub;
		subkey = sub;
		origin = orig;
		channel = ch;

	}
	pfc::string8 get_playing()
	{
		return tr;
		//return playing;
	}

	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused)
	{

	}
	virtual void on_playback_new_track(metadb_handle_ptr track)
	{
		int status;

		if (track.is_empty())
		{
			//do nothing
			return;
		}

		//get track info
		ret = track->format_title_legacy(NULL, tr, "%artist% - %title%", NULL);

		//create new instance of pubnub and send the data to channel
		json_object * msg = json_object_new_object();
		json_object * jstring;

		jstring = json_object_new_string(tr);
		json_object_object_add(msg, "CURRENT_TRACK", jstring);
		
		pubNubPublish(msg);

	}
	virtual void on_playback_stop(play_control::t_stop_reason reason)
	{
		if (reason == 0)
		{
			json_object * msg = json_object_new_object();
			json_object * jstring;

			jstring = json_object_new_string("Nothing Playing!");
			json_object_object_add(msg, "CURRENT_TRACK", jstring);

			pubNubPublish(msg);
		}
		else
		{
			//do nothing
			return;
		}

	}
	virtual void on_playback_seek(double time)
	{
		;
	}
	virtual void on_playback_pause(bool state)
	{
		//todo: fix
		/*
		if (state)
		{
			json_object * msg = json_object_new_object();
			json_object * jstring;

			jstring = json_object_new_string("Track Paused!");
			json_object_object_add(msg, "CURRENT_TRACK", jstring);

			pubNubPublish(msg);

		}
		else
		{


		}
		*/
	}
	virtual void on_playback_edited(metadb_handle_ptr track)
	{
		;
	}
	virtual void on_playback_dynamic_info(const file_info & p_info)
	{
		;
	}
	virtual void on_playback_dynamic_info_track(const file_info & p_info)
	{
		;
	}
	virtual void on_playback_time(double p_time)
	{
		;
	}

	virtual void on_volume_change(float new_val)
	{

	}
};
//static play_callback_static_factory_t<playlist_callback_manager> playlist_callback_factory;



class PubNubTools{
public:
	pubnub_callbacks cb;
	static PubNub *_pn;
	static void *_data;

	playlist_callback_manager* cm;

	//metadb_handle_ptr lm;

	PubNubTools();
	~PubNubTools();
	void RunPubNubSender();
	void PubNubSubscribe();
	int sendPublish(struct json_object* msg);
	void init();
	void ThreadStart();

	//virtual void on_playback_new_track(metadb_handle_ptr p_track) { console::print("New track playing"); };
	
private:
	//static DWORD WINAPI subscribeThread(PubNub);

	std::string pubkey = "";
	std::string subkey = "";
	std::string origin = "";
	std::string channel = "";

	static_api_ptr_t<play_callback_manager> pcm;

	//cm = new playlist_callback_manager;

	//play_callback *pcm;

	//play_callback *m_callback;

	std::string splitQuery(std::string);
	json_object * HandleSearchQuery(std::string);
	json_object * HandleRequestQuery(std::string);
	json_object * HandleVolumeQuery(std::string);
	json_object * HandlePlayControlQuery(std::string query);
	json_object * HandlePlayingQuery();
	json_object * HandleCurrentPlayingQuery(std::string &);
	//void callback_run();
	
};


//This callback is used for getting all objects in the library
class library_manager_search_callback : public main_thread_callback
{

public:
	
	pfc::list_t<metadb_handle_ptr> library;

	pfc::list_t<metadb_handle_ptr> getLibrary(){
		return library;
	}


	void callback_run()
	{	
		static_api_ptr_t<library_manager> lm;
		lm->get_all_items(library);
		t_size n = library.get_count();
		bit_array_bittable deleteMask(n);
	}

};

class playback_control_update_callback : public main_thread_callback
{
public:
	static_api_ptr_t<play_control> pbc;
	int action = -1;

	void setAction(int in){
		// 0 - Volume UP, 1 - Volume DOWN, 2 - MUTE
		action = in;
	}

	int get_result(){
		return action;
	}

	void callback_run()
	{
		//0-2 for volume control
		//3-7 for playback control
		switch (action)
		{
			case 0:
			{
					  pbc->volume_up();
					  action = 0;
			}
				break;
			case 1:
			{
					  pbc->volume_down();
					  action = 0;
			}
				break;
			case 2:
			{
					  pbc->volume_mute_toggle();
					  action = 0;
			}
				break;
			case 3:
			{
				pbc->start();

				action = 0;
			}
				break;
			case 4:
			{
					  if (pbc->is_playing())
						pbc->stop();

					  action = 0;
			}
				break;
			case 5:
			{
					  if (!pbc->is_paused())
						  pbc->pause(true);
					  else
						  pbc->pause(false);

					  action = 0;
			}
				break;
			case 6:
			{
					  if (pbc->is_playing())
						  pbc->previous();

					  action = 0;
			}
			case 7:
			{
					  if (pbc->is_playing())
						  pbc->next();

					  action = 0;
			}
				break;
			default: action = -1;
		}

	}
	
};

class playback_control_current_track_callback : public main_thread_callback
{

public:
	metadb_handle_ptr handleptr = NULL;
	metadb_handle_ptr p_track;
	static_api_ptr_t<play_control> pbc;
	pfc::string8 track = "";

	std::string get_playing(){
		return std::string(track.get_ptr());
		//return playing;
	}

	void callback_run()
	{
		if (pbc->get_now_playing(p_track))
		{
			if (p_track->format_title_legacy(NULL, track, "%artist% - %title%", NULL))
			{
				console::print(track);
				//do nothing
			}
		}
	}
};

class playlist_manager_update_callback : public main_thread_callback
{
public:

	int result = -1;
	playlist_loader::ptr lst;
	abort_callback_dummy m_abort;
	metadb_handle_ptr handle;
	static_api_ptr_t<playlist_manager> plm;
	//std::string request;
	pfc::string request;

	void setRequestString(std::string in){
		//convert request string to pfc::string format for foobar sdk components
		request.set_string(in.c_str());

		//Request comes in as X:\\...\\ ... replace double slashes with single slashes to prevent exception
		request = request.replace("\\\\", "\\");
	}

	int get_result(){
		return result;
	}

	void callback_run()
	{
		console::print(request.get_ptr());

		try
		{
			if (filesystem::g_exists(request.get_ptr(), m_abort))
			{
				static_api_ptr_t<metadb>()->handle_create(handle, make_playable_location(request.get_ptr(), 0));

				metadb_handle_list list; // special list type for metadb_handle_ptr
				list.add_item(handle); // add the handle to the list, not the list to itself

				// plm->activeplaylist_undo_backup(); // allow user to undo the playlist modification
				plm->activeplaylist_add_items(list, bit_array_true()); // p_selection sets the selection state of the added items

				console::print("File open");
				result = 0;

			}
			else
			{
				console::print("File not open");
				result = 1;
			}
		}
		catch (...)
		{
			console::print("There was an error...\r\n");
			result = 2;
		}
	}
};


