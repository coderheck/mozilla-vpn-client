/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"

#include "captiveportal.h"
#include "captiveportaldetectionimpl.h"
#include "captiveportalmonitor.h"
#include "captiveportalnotifier.h"
#include "connectionhealth.h"
#include "controller.h"
#include "frontend/navigator.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"
#include "settingsholder.h"

namespace {
Logger logger("CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection() {
  MZ_COUNT_CTOR(CaptivePortalDetection);
}

CaptivePortalDetection::~CaptivePortalDetection() {
  MZ_COUNT_DTOR(CaptivePortalDetection);
}

void CaptivePortalDetection::initialize() {
  m_active = SettingsHolder::instance()->captivePortalAlert();
  const auto networkWatcher = MozillaVPN::instance()->networkWatcher();
  connect(networkWatcher, &NetworkWatcher::networkChange, this,
          &CaptivePortalDetection::networkChanged);
}

void CaptivePortalDetection::networkChanged() {
  MozillaVPN* vpn = MozillaVPN::instance();

  // The captive portal background monitor is looking
  // wheter a portal on the current network is gone.
  // So it should be deactivated when the network changes.
  // Otherwise we might show the "unblock" notification
  // on networks that never had a portal.
  captivePortalMonitor()->stop();

  ConnectionManager::State state = vpn->connectionManager()->state();
  if (state != ConnectionManager::StateIdle &&
      state != ConnectionManager::StateConnecting &&
      state != ConnectionManager::StateCheckSubscription &&
      state != ConnectionManager::StateConfirming) {
    // Network Changed but we're not connected, no need to test for captive
    // portal
    return;
  }
  logger.debug() << "Current Network Changed, checking for Portal";
  detectCaptivePortal();
}

void CaptivePortalDetection::stateChanged() {
  logger.debug() << "Controller/Stability state changed";

  if (!m_active) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  ConnectionManager::State state = vpn->connectionManager()->state();

  if (state == ConnectionManager::StateOff) {
    // We're not connected yet - start a captivePortal Monitor
    logger.info()
        << "Not connected, starting background captive-portal Monitor";
    captivePortalBackgroundMonitor()->start();
    return;
  }
  logger.info() << "connecting, stopping background captive-portal Monitor";
  captivePortalBackgroundMonitor()->stop();

  if ((state != ConnectionManager::StateIdle ||
       vpn->connectionHealth()->stability() == ConnectionHealth::Stable) &&
      state != ConnectionManager::StateConnecting &&
      state != ConnectionManager::StateCheckSubscription &&
      state != ConnectionManager::StateConfirming) {
    logger.warning() << "No captive portal detection required";
    m_impl.reset();
    // Since we now reached a stable state, on the next time we have an
    // instablity check for portal again.
    m_shouldRun = true;
    return;
  }
  if (!m_shouldRun) {
    logger.debug() << "Captive Portal detection was already done for this "
                      "instability, skipping.";
    return;
  }

  detectCaptivePortal();
}

void CaptivePortalDetection::detectCaptivePortal() {
  logger.debug() << "Start the captive portal detection";

  // Quick return in case this method is called by the inspector even when the
  // feature is disabled.
  if (!m_active) {
    return;
  }
  captivePortalBackgroundMonitor()->maybeCheck();

  // The monitor must be off when detecting the captive portal.
  captivePortalMonitor()->stop();

  MozillaVPN* vpn = MozillaVPN::instance();

  // This method is called by the inspector too. Let's check the status of the
  // VPN.
  ConnectionManager::State state = vpn->connectionManager()->state();
  if (state != ConnectionManager::StateIdle &&
      state != ConnectionManager::StateConnecting &&
      state != ConnectionManager::StateConfirming) {
    logger.warning() << "The VPN is not online. Ignore request.";
    return;
  }

  logger.debug() << "Captive portal detection started";

#if defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS)
  m_impl.reset(new CaptivePortalDetectionImpl());
#else
  logger.warning()
      << "This platform does not support captive portal detection yet";
  return;
#endif

  Q_ASSERT(m_impl);

  connect(m_impl.get(), &CaptivePortalDetectionImpl::detectionCompleted, this,
          &CaptivePortalDetection::detectionCompleted);

  m_impl->start();
}

void CaptivePortalDetection::settingsChanged() {
  logger.debug() << "Settings has changed";
  m_active = SettingsHolder::instance()->captivePortalAlert();

  if (!m_active) {
    captivePortalMonitor()->stop();
    captivePortalBackgroundMonitor()->stop();
    m_impl.reset();
  }
}

void CaptivePortalDetection::detectionCompleted(
    CaptivePortalRequest::CaptivePortalResult detected) {
  logger.debug() << "Detection completed:" << detected;
  m_impl.reset();
  m_shouldRun = false;
  switch (detected) {
    case CaptivePortalRequest::CaptivePortalResult::NoPortal:
    case CaptivePortalRequest::CaptivePortalResult::Failure:
      return;
    case CaptivePortalRequest::CaptivePortalResult::PortalDetected:
      captivePortalDetected();
      return;
  }
}

void CaptivePortalDetection::captivePortalDetected() {
  logger.debug() << "Captive portal detected!";

  // Quick return in case this method is called by the inspector even when the
  // feature is disabled.
  if (!m_active) {
    return;
  }
  emit captivePortalPresent();
  Navigator::instance()->requestScreen(MozillaVPN::ScreenCaptivePortal);

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->connectionManager()->state() == ConnectionManager::StateIdle) {
    captivePortalNotifier()->notifyCaptivePortalBlock();
  }
}

void CaptivePortalDetection::captivePortalGone() {
  logger.debug() << "Portal gone";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == App::StateMain &&
      vpn->connectionManager()->state() == ConnectionManager::StateOff) {
    captivePortalNotifier()->notifyCaptivePortalUnblock();
    captivePortalMonitor()->stop();
  }
}

void CaptivePortalDetection::deactivationRequired() {
  logger.debug() << "The user wants to deactivate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->connectionManager()->state() != ConnectionManager::StateOff) {
    vpn->deactivate();
    captivePortalMonitor()->start();
  }
}

void CaptivePortalDetection::activationRequired() {
  logger.debug() << "User wants to activate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == App::StateMain &&
      vpn->connectionManager()->state() == ConnectionManager::StateOff) {
    vpn->connectionManager()->captivePortalGone();
    vpn->activate();
  }
}

CaptivePortalMonitor* CaptivePortalDetection::captivePortalMonitor() {
  if (!m_captivePortalMonitor) {
    m_captivePortalMonitor = new CaptivePortalMonitor(this);

    connect(m_captivePortalMonitor, &CaptivePortalMonitor::online, this,
            &CaptivePortalDetection::captivePortalGone);
  }

  return m_captivePortalMonitor;
}

CaptivePortalMonitor* CaptivePortalDetection::captivePortalBackgroundMonitor() {
  if (!m_captivePortalBackgroundMonitor) {
    m_captivePortalBackgroundMonitor = new CaptivePortalMonitor(this);

    connect(m_captivePortalBackgroundMonitor, &CaptivePortalMonitor::online,
            MozillaVPN::instance()->connectionManager(),
            &ConnectionManager::captivePortalGone);
    connect(m_captivePortalBackgroundMonitor, &CaptivePortalMonitor::offline,
            MozillaVPN::instance()->connectionManager(),
            &ConnectionManager::captivePortalPresent);
  }

  return m_captivePortalBackgroundMonitor;
}

CaptivePortalNotifier* CaptivePortalDetection::captivePortalNotifier() {
  if (!m_captivePortalNotifier) {
    m_captivePortalNotifier = new CaptivePortalNotifier(this);

    connect(m_captivePortalNotifier, &CaptivePortalNotifier::activationRequired,
            this, &CaptivePortalDetection::activationRequired);
  }

  return m_captivePortalNotifier;
}
