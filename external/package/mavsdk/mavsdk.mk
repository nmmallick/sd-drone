################################################################################
#
# mavsdk
#
################################################################################
MAVSDK_VERSION = b29239df34afa4bfc227b73e98d06c28c264dfa7
MAVSDK_SITE = git@github.com:mavlink/MAVSDK.git
MAVSDK_SITE_METHOD = git
MAVSDK_INSTALL_STAGING = YES
MAVSDK_CONF_OPTS = -DBUILD_TESTS=OFF

$(eval $(cmake-package))
