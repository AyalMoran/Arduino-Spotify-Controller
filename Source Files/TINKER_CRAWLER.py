

from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.common.by import By
import time
import spotipy
from spotipy.oauth2 import SpotifyOAuth

YOUR_CLIENT_ID = 'YOUR_CLIENT_ID'
YOUR_CLIENT_SECRET = 'YOUR_CLIENT_SECRET'
YOUR_REDIRECT_URI = 'http://localhost:8000/callback'
sp = spotipy.Spotify(auth_manager=SpotifyOAuth(client_id=YOUR_CLIENT_ID,
                                               client_secret=YOUR_CLIENT_SECRET,
                                               redirect_uri=YOUR_REDIRECT_URI,
                                               scope='app-remote-control user-modify-playback-state user-read-playback-state'))


# Setup Chrome options
chrome_options = webdriver.ChromeOptions()
# chrome_options.add_argument("--headless")  # Commented this line
chrome_options.add_argument("--no-sandbox")
chrome_options.add_argument("--disable-dev-shm-usage")

# Set path to chromedriver as per your configuration
webdriver_service = Service(ChromeDriverManager().install())

# Choose Chrome Browser
driver = webdriver.Chrome(service=webdriver_service, options=chrome_options)

driver.get('https://www.tinkercad.com/things/imeaXwLdgUI-arduino-spotify-control/editel?sharecode=suJjARiHyt01tgR90r23L7j3Dfr_yBiEUWLanHvIQrY')


# In[ ]:


def print_current_track():
    playback = sp.current_playback()
    if playback and playback['item']:
        track = playback['item']
        artist = track['artists'][0]['name']
        track_name = track['name']
        print(f"Now Playing: {artist} - {track_name}")
    else:
        print("No track is currently playing")


volume_change = 25

line_count = 0

while True:
    # find the element using CSS selector
    serial_data = driver.find_element(By.CSS_SELECTOR, '.code_panel__serial__content.js-code_panel__serial__content')

    # Get the current text and split into lines
    current_text = serial_data.text.split('\n')

    # Filter out "Clear" and "Send" from the lines
    current_text = [line for line in current_text if line not in ['Clear', 'Send']]

    # If there are new lines, print them
    controlParameter = 0
    for i in range(line_count, len(current_text)):
        #print(current_text[i])
        controlParameter = current_text[i]


    # Update line_count to the number of lines in current_text
    line_count = len(current_text)

    time.sleep(1) 
    
    # resume/start
    if controlParameter == 'PLAY':
        # Check if something is already playing
        current_playback = sp.current_playback()
        if current_playback is None or not current_playback['is_playing']:
            sp.start_playback()
            print("Playback Resumed")
        else:
            print("A song is already playing")
        time.sleep(0.5)
        print_current_track()

    # pause
    elif controlParameter == 'STOP':
        # Check if something is playing
        current_playback = sp.current_playback()
        if current_playback is not None and current_playback['is_playing']:
            sp.pause_playback()
            print("Playback Paused")
        else:
            print("No song is currently playing")
        time.sleep(0.5)
        print_current_track()

    #prev
    elif controlParameter == 'PREV':
        sp.previous_track()
        print("prev track")
        time.sleep(0.5)
        print_current_track()
    
    #next
    elif controlParameter == 'NEXT':
        sp.next_track()
        print("Next Track")
        time.sleep(0.5)
        print_current_track()

    elif controlParameter == 'VOLDOWN':
        # Get the current volume level
        playback = sp.current_playback()
        current_volume = playback["device"]["volume_percent"]
        new_volume = max(current_volume - volume_change, 0)
        sp.volume(new_volume)
        print("Volume Down")
        print_current_track()

    elif controlParameter == 'VOLUP':
        # Get the current volume level
        playback = sp.current_playback()
        current_volume = playback["device"]["volume_percent"]
        new_volume = min(current_volume + volume_change, 100)
        sp.volume(new_volume)
        print("Volume Up")
        print_current_track()





