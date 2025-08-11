from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from webdriver_manager.chrome import ChromeDriverManager
import spotipy
from spotipy.oauth2 import SpotifyOAuth
import time


# ==============================
# Spotify API Configuration
# ==============================
YOUR_CLIENT_ID = 'YOUR_CLIENT_ID'
YOUR_CLIENT_SECRET = 'YOUR_CLIENT_SECRET'
YOUR_REDIRECT_URI = 'http://localhost:8000/callback'

sp = spotipy.Spotify(
    auth_manager=SpotifyOAuth(
        client_id=YOUR_CLIENT_ID,
        client_secret=YOUR_CLIENT_SECRET,
        redirect_uri=YOUR_REDIRECT_URI,
        scope='app-remote-control user-modify-playback-state user-read-playback-state'
    )
)

# ==============================
# Selenium WebDriver Setup
# ==============================
chrome_options = webdriver.ChromeOptions()
chrome_options.add_argument("--no-sandbox")
chrome_options.add_argument("--disable-dev-shm-usage")

webdriver_service = Service(ChromeDriverManager().install())
driver = webdriver.Chrome(service=webdriver_service, options=chrome_options)

driver.get(
    'https://www.tinkercad.com/things/imeaXwLdgUI-arduino-spotify-control/editel?sharecode=suJjARiHyt01tgR90r23L7j3Dfr_yBiEUWLanHvIQrY'
)

# ==============================
# Helper Functions
# ==============================
def print_current_track():
    playback = sp.current_playback()
    if playback and playback['item']:
        track = playback['item']
        artist = track['artists'][0]['name']
        track_name = track['name']
        print(f"Now Playing: {artist} - {track_name}")
    else:
        print("No track is currently playing")


# ==============================
# Main Control Loop
# ==============================
volume_change = 25
line_count = 0

while True:
    # Get serial output from TinkerCAD
    serial_data = driver.find_element(
        By.CSS_SELECTOR, '.code_panel__serial__content.js-code_panel__serial__content'
    )
    current_text = serial_data.text.split('\n')

    # Remove unwanted UI text
    current_text = [line for line in current_text if line not in ['Clear', 'Send']]

    # Determine the latest control parameter
    control_parameter = None
    for i in range(line_count, len(current_text)):
        control_parameter = current_text[i]

    line_count = len(current_text)
    time.sleep(1)

    # ==============================
    # Command Handling
    # ==============================
    if control_parameter == 'PLAY':
        current_playback = sp.current_playback()
        if current_playback is None or not current_playback['is_playing']:
            sp.start_playback()
            print("Playback Resumed")
        else:
            print("A song is already playing")
        time.sleep(0.5)
        print_current_track()

    elif control_parameter == 'STOP':
        current_playback = sp.current_playback()
        if current_playback and current_playback['is_playing']:
            sp.pause_playback()
            print("Playback Paused")
        else:
            print("No song is currently playing")
        time.sleep(0.5)
        print_current_track()

    elif control_parameter == 'PREV':
        sp.previous_track()
        print("Previous Track")
        time.sleep(0.5)
        print_current_track()

    elif control_parameter == 'NEXT':
        sp.next_track()
        print("Next Track")
        time.sleep(0.5)
        print_current_track()

    elif control_parameter == 'VOLDOWN':
        playback = sp.current_playback()
        if playback:
            current_volume = playback["device"]["volume_percent"]
            new_volume = max(current_volume - volume_change, 0)
            sp.volume(new_volume)
            print("Volume Down")
            print_current_track()

    elif control_parameter == 'VOLUP':
        playback = sp.current_playback()
        if playback:
            current_volume = playback["device"]["volume_percent"]
            new_volume = min(current_volume + volume_change, 100)
            sp.volume(new_volume)
            print("Volume Up")
            print_current_track()
