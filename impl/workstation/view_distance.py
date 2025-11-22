#!/usr/bin/env python3
"""View distance server status and data"""
import urllib.request
import json
import time

def format_timestamp(ms_timestamp):
    """Format millisecond timestamp to readable string"""
    if ms_timestamp is None:
        return "Never"
    sec = ms_timestamp / 1000
    return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sec)) + f".{int(ms_timestamp % 1000):03d}"

try:
    # Query the server via HTTP
    req = urllib.request.Request('http://localhost:8080/status')
    with urllib.request.urlopen(req, timeout=2) as response:
        data = json.loads(response.read().decode('utf-8'))
    
    print("=" * 60)
    print("Distance Server Status")
    print("=" * 60)
    print(f"Port: 8080")
    print(f"Server: Running")
    print()
    print("Distance Data:")
    print(f"  Total measurements: {data['total_measurements']}")
    if data['total_measurements'] > 0:
        print(f"  Most recent distance: {data['most_recent']:.2f} meters")
        print(f"  Average distance: {data['average']:.2f} meters")
        print(f"  Last measurement time: {format_timestamp(data['last_time'])}")
        
        # Show statistics if available
        if data.get('statistics'):
            stats = data['statistics']
            print()
            print("  Statistics:")
            print(f"    Minimum: {stats.get('min', 'N/A'):.2f} meters" if stats.get('min') is not None else "    Minimum: N/A")
            print(f"    Maximum: {stats.get('max', 'N/A'):.2f} meters" if stats.get('max') is not None else "    Maximum: N/A")
            print(f"    Median: {stats.get('median', 'N/A'):.2f} meters" if stats.get('median') is not None else "    Median: N/A")
        
        if data['recent_measurements']:
            print()
            print("  Recent measurements (last 10):")
            recent_with_time = data.get('measurements_with_time', [])[-10:]
            for i, m in enumerate(recent_with_time, 1):
                interval_str = f" (+{m['time_since_previous']:.2f}s)" if m.get('time_since_previous') is not None else " (first)"
                print(f"    {i}. {m['distance']:.2f} meters{interval_str}")
        
        # Option to show all measurements with time intervals
        if data.get('measurements_with_time'):
            print()
            print(f"  All measurements with time intervals ({len(data['measurements_with_time'])} total):")
            for i, m in enumerate(data['measurements_with_time'], 1):
                timestamp_str = format_timestamp(m['timestamp'])
                interval_str = f" | Interval: +{m['time_since_previous']:.2f}s" if m.get('time_since_previous') is not None else " | Interval: first"
                print(f"    {i:3d}. {m['distance']:.2f} meters | Time: {timestamp_str}{interval_str}")
    else:
        print("  No measurements received yet")
    print("=" * 60)
except urllib.error.URLError as e:
    print("=" * 50)
    print("Distance Server Status")
    print("=" * 50)
    print(f"Error: Could not connect to server on port 8080")
    print(f"  {e}")
    print()
    print("Make sure the server is running:")
    print("  systemctl --user status deauth-lock.service")
    print("=" * 50)
except Exception as e:
    print(f"Error: {e}")

