
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const vpn = require('./helper.js');
 const queries = require('./queries.js');
 const { startAndConnect } = require('./setupVpn.js');

describe("subscription needed tests", function() {
  // TODO: Create tests for this screen.

  describe('subscription needed related telemetry tests', function() {
    if(vpn.runningOnWasm()) {
      return;
    }

    this.ctx.guardianOverrideEndpoints = {
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          body: {
            user: {
              avatar: '',
              display_name: 'Test',
              email: 'test@mozilla.com',
              max_devices: 5,
              // This is the important bit.
              // The user data has no active subscriptions.
              subscriptions: {},
              devices: [],
            },
            token: 'our-token'
          }
        }
      }
    };

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?share_link_id=228137467679

    describe('subscription screen', function() {
      const screen = "subscription_needed";

      beforeEach(async () => {
        await vpn.authenticateInApp();
      });

      it("impression event is recorded", async () => {
        const subscriptionNeededViewEvent = await vpn.getOneEventOfType({
          eventCategory: "sample",
          eventName: "appStep"
        });
        const subscriptionNeededViewEventExtras = subscriptionNeededViewEvent.extra;
        assert.strictEqual(screen, subscriptionNeededViewEventExtras.screen);
        assert.strictEqual("impression", subscriptionNeededViewEventExtras.action);
      })

      it("get help event is recorded", async () => {
        // Click the "Get help" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_GET_HELP.visible());
        await vpn.testLastInteractionEvent({
            eventName: "getHelp",
            elementId: "get_help",
            screen
        });
      });

      it("legal disclaimer events are recorded", async () => {
        // Click the "Terms of Service" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_TERMS_OF_SERVICE.visible());
        await vpn.testLastInteractionEvent({
            eventName: "termsOfService",
            elementId: "terms_of_service",
            screen
        });

        // Click the "Privacy Notice" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_PRIVACY_NOTICE.visible());
        await vpn.testLastInteractionEvent({
            eventName: "privacyNotice",
            elementId: "privacy_notice",
            screen
        });
      });

      it("sign out event is recorded", async () => {
        // Click the "Sign out" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_SIGN_OUT.visible());
        await vpn.testLastInteractionEvent({
            eventCategory: "sample",
            eventName: "iapSubscriptionFailed",
            elementId: "sign_out",
            screen
        });
      });

      it("subscribe now button event is recorded", async () => {
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // When we click the "Subscribe now" button,
        // that records an app_step for clicking on the button
        // and then immediatelly goes on to the loading screen.
        //
        // App step is also recorded on every internal state change,
        // and clicking the button prompts a state change. Therefore
        // the last event is the state change event and we want the
        // _next to last_ event instead.
        const subscriptionNeededViewEvent = await vpn.getOneEventOfType({
          eventCategory: "sample",
          eventName: "appStep"
        }, 1);
        const subscriptionNeededViewEventExtras = subscriptionNeededViewEvent.extra;
        assert.strictEqual(screen, subscriptionNeededViewEventExtras.screen);
        assert.strictEqual("select", subscriptionNeededViewEventExtras.action);
        assert.strictEqual("subscribe_now", subscriptionNeededViewEventExtras.element_id);
      });

      it("restore purchase event is recorded", async () => {
        // Make the "Already a subscriber?" button visible
        await vpn.setQueryProperty(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE, "visible", true)
        // Click the "Already a subscriber?" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE.visible());
        await vpn.testLastInteractionEvent({
            eventCategory: "sample",
            eventName: "iapRestoreSubStarted",
            elementId: "already_a_subscriber",
            screen
        });
      });
    });

    describe('loading screens', function() {
      const inBrowserScreen = "subscribe_in_browser"

      it("impression event is recorded for in browser screen", async () => {
        await vpn.authenticateInApp();
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Wait for the loading screen to show up
        await vpn.waitForQuery(queries.screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_VIEW.visible());

        const subscriptionNeededViewEvent = await vpn.getOneEventOfType({
          eventCategory: "impression",
          eventName: "subInBrowserScreenView",
          expectedEventCount: 1
        });
        assert.strictEqual(subscriptionNeededViewEvent.extra.action, "impression");
        assert.strictEqual(subscriptionNeededViewEvent.extra.screen, inBrowserScreen);
      });

      // TODO (VPN-4784, VPN-4783): This cannot be tested util we are able to run
      // functional tests in mobile. In app purchase screens are mobile only.
      it.skip("impression event is recorded for in app screen");

      it("cancel button event is recorded (in browser only)", async () => {
        await vpn.authenticateInApp();
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Click on the "Cancel" button
        await vpn.waitForQueryAndClick(queries.screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_CANCEL.visible());
        await vpn.testLastInteractionEvent({
          eventCategory: "sample",
          eventName: "iapSubscriptionFailed",
          elementId: "cancel",
          screen: inBrowserScreen
        });
      });
    });
  });
});
