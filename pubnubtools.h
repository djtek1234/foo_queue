
#include "pubnub.h"
#include "pubnub.hpp"
#include "pubnub-sync.h"
#include "pubnub-sync.hpp"
#include "pubnub-priv.h"
#include "pubnub-libevent.h"

#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"

#undef pubnub_init
#undef pubnub_done
#undef pubnub_publish
#undef pubnub_subscribe
#undef pubnub_subscribe_multi
#undef pubnub_history
#undef pubnub_here_now
#undef pubnub_time

//Callback to handle playback events
class playlist_callback_manager : public play_callback
{
public:

	pfc::string8 tr = "";
	std::string pk = "";
	std::string sk = "";
	std::string on = "";
	std::string cn = "";
	bool ret;

	void pubNubPublish(struct json_object* msg)
	{
		pubnub_sync *sync = pubnub_sync_init();

		PubNub pn(pk, sk, &pubnub_sync_callbacks, sync);
		pn.set_origin(on);
		pn.publish(cn, *msg, -1, NULL);
	}

	void setKeys(std::string pub, std::string sub, std::string orig, std::string ch)
	{
		pk = pub;
		sk = sub;
		on = orig;
		cn = ch;

	}
	pfc::string8 get_playing()
	{
		return tr;
	}

	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused)
	{

	}
	virtual void on_playback_new_track(metadb_handle_ptr track)
	{
		int status;

		if (track.is_empty())
		{
			//no track playing so return
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


class PubNubTools{
public:
	pubnub_callbacks cb;
	static PubNub *_pn;
	static void *_data;

	playlist_callback_manager* cm;

	PubNubTools();
	~PubNubTools();
	void RunPubNubSender();
	void PubNubSubscribe();
	int sendPublish(struct json_object* msg);
	//void init();
	//void ThreadStart();

private:
	std::string pubkey = "";
	std::string subkey = "";
	std::string origin = "";
	std::string channel = "";

	static_api_ptr_t<play_callback_manager> pcm;


	std::string splitQuery(std::string);

	json_object * HandleSearchQuery(std::string);
	json_object * HandleRequestQuery(std::string);
	json_object * HandleVolumeQuery(std::string);
	json_object * HandlePlayControlQuery(std::string query);
	json_object * HandlePlayingQuery();
	//json_object * HandleCurrentPlayingQuery(std::string &);
	json_object * HandlePlaylistQuery();
	json_object * HandlePlayQuery(std::string);
	json_object * HandleRemoveQuery(std::string);

};


//Callback to handle playback control actions
class playback_control_update_callback : public main_thread_callback
{
public:
	static_api_ptr_t<play_control> pbc;
	int action = -1;

	void setAction(int in)
	{
		// 0 - Volume UP, 1 - Volume DOWN, 2 - MUTE, 3 - PLAY, 4 - STOP, 5 - PAUSE, 6 - PREVIOUS, 7 - NEXT
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

//Callback to get the current playing track if applicable
class playback_control_current_track_callback : public main_thread_callback
{
public:
	metadb_handle_ptr p_track;
	static_api_ptr_t<play_control> pbc;
	pfc::string8 track = "";

	std::string get_playing()
	{
		return std::string(track.get_ptr());
	}

	void callback_run()
	{
		if (pbc->get_now_playing(p_track))
		{
			if (p_track->format_title_legacy(NULL, track, "%artist% - %title%", NULL))
			{
				//do nothing...track has been set 
			}
		}
	}
};

//Callback to add a new song to the active playlist
class playlist_manager_update_callback : public main_thread_callback
{
public:

	int result = -1;
	abort_callback_dummy m_abort;
	metadb_handle_ptr handle;
	static_api_ptr_t<playlist_manager> plm;
	pfc::string request;

	void setRequestString(std::string in)
	{
		//convert request string to pfc::string format for foobar sdk components
		request.set_string(in.c_str());

		//Request comes in as X:\\...\\ ... replace double slashes with single slashes to prevent exception
		request = request.replace("\\\\", "\\");
	}

	int get_result()
	{
		return result;
	}

	void callback_run()
	{
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
				//file does not exist
				result = 1;
			}
		}
		catch (...)
		{
			//exception
			console::print("There was an error...\r\n");
			result = 2;
		}
	}
};

//Callback to play an item that is loaded within the active playlist
class playlist_manager_play_callback : public main_thread_callback
{
public:

	int result = -1;
	static_api_ptr_t<playlist_manager> plm;
	static_api_ptr_t<play_control> pbc;
	pfc::string8 track;
	pfc::list_t<metadb_handle_ptr> playlist_tracks;
	t_size total_tracks = 0;

	void setTrackString(std::string in)
	{
		//convert request string to pfc::string format for foobar sdk components
		track.set_string(in.c_str());
	}

	int get_result()
	{
		return result;
	}

	void callback_run()
	{
		plm->activeplaylist_get_all_items(playlist_tracks);
		total_tracks = plm->activeplaylist_get_item_count();
		pfc::string8 tr = "";
		t_size active_playlist = plm->get_active_playlist();

		t_size location;
		for (int i = 0; i < total_tracks; i++)
		{
			playlist_tracks[i]->format_title_legacy(NULL, tr, "%artist% - %title%", NULL);

			//track exists in playlist...play it
			if (tr == track)
			{
				plm->activeplaylist_find_item(playlist_tracks[i], location);
				plm->queue_add_item_playlist(active_playlist, location);
				pbc->start(pbc->track_command_play, false);
				result = 0;
			}
		}

	}
};

//Callback to remove an item from the active playlist
class playlist_manager_remove_callback : public main_thread_callback
{
public:

	int result = -1;
	static_api_ptr_t<playlist_manager> plm;
	static_api_ptr_t<play_control> pbc;
	pfc::string8 track;
	pfc::list_t<metadb_handle_ptr> playlist_tracks;
	t_size total_tracks = 0;

	void setTrackString(std::string in)
	{
		//convert request string to pfc::string format for foobar sdk components
		track.set_string(in.c_str());
	}

	int get_result()
	{
		return result;
	}

	void callback_run()
	{
		plm->activeplaylist_get_all_items(playlist_tracks);
		total_tracks = plm->activeplaylist_get_item_count();
		pfc::string8 tr = "";
		t_size active_playlist = plm->get_active_playlist();

		t_size location;
		for (int i = 0; i < total_tracks; i++)
		{
			playlist_tracks[i]->format_title_legacy(NULL, tr, "%artist% - %title%", NULL);

			if (tr == track)
			{
				const bit_array_one loc(i);

				plm->playlist_remove_items(active_playlist, loc);
				//plm->activeplaylist_find_item(playlist_tracks[i], location);
				/*
				if (plm->playlist_remove_items(active_playlist, items))
				{
				result = 0;
				}
				else
				{
				result = 1;
				}
				*/

			}
		}
	}

};

//Callback to retrieve all of the tracks within the active playlist
class playlist_manager_contents_callback : public main_thread_callback
{
public:
	int result = -1;
	pfc::list_t<metadb_handle_ptr> items;
	static_api_ptr_t<playlist_manager> plm;

	int get_result()
	{
		return result;
	}

	pfc::list_t<metadb_handle_ptr> get_playlist(){
		return items;
	}

	void callback_run()
	{

		try
		{
			plm->activeplaylist_get_all_items(items);
			result = 1;

		}
		catch (...)
		{
			console::print("There was an error...\r\n");
			result = 2;
		}
	}
};

//This callback is used for getting all objects in the library
class library_manager_search_callback : public main_thread_callback
{
public:
	pfc::list_t<metadb_handle_ptr, pfc::alloc_fast_aggressive> library;
	static_api_ptr_t<library_manager> lm;
	bool libraryPresent = true;

	metadb_handle_list getLibrary()
	{
		return library;
	}

	bool checkLibrary()
	{
		return libraryPresent;
	}

	void callback_run()
	{	
		try
		{
			libraryPresent = lm->is_library_enabled();

			if (libraryPresent)
			{
				lm->get_all_items(library);
			}

		}
		catch (...)
		{
			console::print("There was an error...\r\n");
		}
	}

};