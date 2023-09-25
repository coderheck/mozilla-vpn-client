/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');


describe('Initialize', function() {
  it('Navigating to and from the help menu is possible', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);

    await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
  });

  it('SwipeView is visible', async () => {
    await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
  });

  it('Sign up button is visible', async () => {
    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Already a subscriber button is visible', async () => {
    await vpn.waitForQuery(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
  });

  it('Panel title is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await vpn.wait();
       await vpn.waitForQuery(queries.screenInitialize.PANEL_TITLE.visible());
       assert.equal(
           await vpn.getQueryProperty(
               queries.screenInitialize.PANEL_TITLE, 'text'), 'Mozilla VPN');
     });

  it('Panel description is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await vpn.wait();
       const descriptionText = await vpn.getQueryProperty(
           queries.screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Firefox'));
     });

  it('Panel title and description are updated when SwipeView currentIndex changes',
     async () => {
       await vpn.waitForQuery(queries.screenInitialize.SWIPE_VIEW.visible());
       await vpn.setQueryProperty(
           queries.screenInitialize.SWIPE_VIEW, 'currentIndex', 2);
       await vpn.wait();
       assert.equal(
           await vpn.getQueryProperty(
               queries.screenInitialize.PANEL_TITLE.visible(), 'text'),
           'One tap to safety');
       const descriptionText = await vpn.getQueryProperty(
           queries.screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Protecting yourself is simple'));
     });

  it('Sign up button opens auth flow', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.SIGN_UP_BUTTON.visible());
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
  });

  it('Already a subscriber? opens auth flow', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUsT.visible());
  });

  describe('initialize related telemetry tests', () => {
    if(vpn.runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?userEmail=sandrigo@mozilla.com&openComment=3458764559943493792&mid=8418131&utm_source=notification&utm_medium=email&utm_campaign=mentions&utm_content=reply-to-mention&share_link_id=496680579489

    it("impression events are recorded and contain the right screen id", async () => {
      const numberOfSlides = await vpn.getQueryProperty(
        queries.screenInitialize.SWIPE_VIEW_REPEATER, 'slidesCount');

      assert(numberOfSlides > 0);

      let currentIndex = 0;
      do {
        if (currentIndex > 0) {
          await vpn.setQueryProperty(
            queries.screenInitialize.SWIPE_VIEW, 'currentIndex', currentIndex);
        }

        const screenId = await vpn.getQueryProperty(
          queries.screenInitialize.SWIPE_VIEW, 'telemetryScreenId');
        assert(screenId);

        const signupScreenEvents = await vpn.gleanTestGetValue("impression", "signupScreenView", "main");
        assert.strictEqual(signupScreenEvents.length, currentIndex + 1);
        const signupScreenExtras = signupScreenEvents[currentIndex].extra;
        assert.strictEqual(screenId, signupScreenExtras.screen);
        assert.strictEqual("impression", signupScreenExtras.action);

        currentIndex++;
      } while(currentIndex < numberOfSlides);
    });

    it("get help click events are recorded and contain the right screen id", async () => {
      const numberOfSlides = await vpn.getQueryProperty(
        queries.screenInitialize.SWIPE_VIEW_REPEATER, 'slidesCount');

      assert(numberOfSlides > 0);

      let currentIndex = 0;
      do {
        if (currentIndex > 0) {
          await vpn.setQueryProperty(
            queries.screenInitialize.SWIPE_VIEW, 'currentIndex', currentIndex);
        }

        const screenId = await vpn.getQueryProperty(
          queries.screenInitialize.SWIPE_VIEW, 'telemetryScreenId');
        assert(screenId);

        // Click on the "Get help" link
        await vpn.waitForQueryAndClick(
          queries.screenInitialize.GET_HELP_LINK.visible());
        const getHelpEvents = await vpn.gleanTestGetValue("interaction", "getHelp", "main");
        // We will click on this button once per interation
        assert.strictEqual(getHelpEvents.length, currentIndex + 1);
        const getHelpExtras = getHelpEvents[currentIndex].extra;
        assert.strictEqual(screenId, getHelpExtras.screen);
        assert.strictEqual("select", getHelpExtras.action);
        assert.strictEqual("get_help", getHelpExtras.element_id);
        // Return to initialize
        await vpn.waitForQueryAndClick(
          queries.screenGetHelp.BACK_BUTTON.visible());

        currentIndex++;
      } while(currentIndex < numberOfSlides);
    });

    it("sign up click events are recorded and contain the right screen id", async () => {
      const numberOfSlides = await vpn.getQueryProperty(
        queries.screenInitialize.SWIPE_VIEW_REPEATER, 'slidesCount');

      assert(numberOfSlides > 0);

      let currentIndex = 0;
      do {
        if (currentIndex > 0) {
          await vpn.setQueryProperty(
            queries.screenInitialize.SWIPE_VIEW, 'currentIndex', currentIndex);
        }

        const screenId = await vpn.getQueryProperty(
          queries.screenInitialize.SWIPE_VIEW, 'telemetryScreenId');
        assert(screenId);

        // Click the "Sign up" button
        await vpn.waitForQueryAndClick(
          queries.screenInitialize.SIGN_UP_BUTTON.visible());
        const signupButtonEvents = await vpn.gleanTestGetValue("interaction", "onboardingCtaClick", "main");
        assert.strictEqual(signupButtonEvents.length, currentIndex + 1);
        const signupButtonExtras = signupButtonEvents[currentIndex].extra;
        assert.strictEqual(screenId, signupButtonExtras.screen);
        assert.strictEqual("select", signupButtonExtras.action);
        assert.strictEqual("sign_up", signupButtonExtras.element_id);
        // Return to initialize
        await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());

        currentIndex++;
      } while(currentIndex < numberOfSlides);
    });

    it("already a subscriber click events are recorded and contain the right screen id", async () => {
      const numberOfSlides = await vpn.getQueryProperty(
        queries.screenInitialize.SWIPE_VIEW_REPEATER, 'slidesCount');

      assert(numberOfSlides > 0);

      let currentIndex = 0;
      do {
        if (currentIndex > 0) {
          await vpn.setQueryProperty(
            queries.screenInitialize.SWIPE_VIEW, 'currentIndex', currentIndex);
        }

        const screenId = await vpn.getQueryProperty(
          queries.screenInitialize.SWIPE_VIEW, 'telemetryScreenId');
        assert(screenId);

        // Click the "Already a subscriber?" button
        await vpn.waitForQueryAndClick(
          queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
        const alreadyASubscriberButtonEvents = await vpn.gleanTestGetValue("interaction", "onboardingCtaClick", "main");
        assert.strictEqual(alreadyASubscriberButtonEvents.length, currentIndex + 1);
        const alreadyASubscriberButtonExtras = alreadyASubscriberButtonEvents[currentIndex].extra;
        assert.strictEqual(screenId, alreadyASubscriberButtonExtras.screen);
        assert.strictEqual("select", alreadyASubscriberButtonExtras.action);
        assert.strictEqual("already_a_subscriber", alreadyASubscriberButtonExtras.element_id);
        // Return to initialize
        await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());

        currentIndex++;
      } while(currentIndex < numberOfSlides);
    });
  });
});
