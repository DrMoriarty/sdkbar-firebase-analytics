# Description

This plugin adds firebase analytics integration.

# Installation

`sdkbar -i https://github.com/OrangeAppsRu/sdkbar-firebase-analytics`

# Dependencies

- sdkbar-utils (https://github.com/OrangeAppsRu/sdkbar-utils)
- sdkbar-firebase (https://github.com/OrangeAppsRu/sdkbar-firebase)

# Plugin JS interface

- `analytics.init()`
- `analytics.logEvent(event_name)`
- `analytics.logEvent(event_name, parameters_dictionary)`
- `analytics.logEvent(event_name, parameter, value)`
- `analytics.setScreenName(screen_name, screen_class)`
- `analytics.setUserID(user_id)`
- `analytics.setUserProperty(property_name, property_value)`
