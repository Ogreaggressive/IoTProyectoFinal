// Include the Alexa SDK v2
const Alexa = require("ask-sdk-core");
const AWS = require('aws-sdk');
const IoTData = new AWS.IotData({endpoint:'endpoint' });

const OpenRollerParams = {
    topic: '$aws/things/cosin/shadow/update',
    payload: '{"state": {"desired": {"curtainState": "CURTAIN_OPENED"}}}',
    qos: 0
};

const CloseRollerParams = {
    topic: '$aws/things/cosin/shadow/update',
    payload: '{"state": {"desired": {"curtainState": "CURTAIN_CLOSED"}}}',
    qos: 0
};

const detectLightTrueParams = {
    topic: '$aws/things/cosin/shadow/update',
    payload: '{"state": {"desired": {"detectLight": true}}}',
    qos: 0
};

const detectLightFalseParams = {
    topic: '$aws/things/cosin/shadow/update',
    payload: '{"state": {"desired": {"detectLight": false}}}',
    qos: 0
};

const ShadowParams = {
  thingName: 'cosin'
};

function getShadowPromise(params) {
  return new Promise((resolve, reject) => {
    IoTData.getThingShadow(params, (err, data) => {
      if (err) {
        console.log(err, err.stack);
        reject(`Failed to get thing shadow: ${err.errorMessage}`);
      } else {
        resolve(JSON.parse(data.payload));
      }
    });
  });
}

const LaunchRequestHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'LaunchRequest';
  },
  handle(handlerInput) {
    const speechText = 'Greetings, Would you like dinner or a bath or ... me ?';

    return handlerInput.responseBuilder
      .speak(speechText)
      .reprompt(speechText)
      .getResponse();
  }
};

const OpenRollerIntentHandler = {
      canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
          && Alexa.getIntentName(handlerInput.requestEnvelope) === 'openRoller';
      },
      async handle(handlerInput) {
        let curtainState;
        let speechText;
        await getShadowPromise(ShadowParams)
        .then((result) => curtainState = result.state.reported.curtainState);
        console.log('TurnOnIntentHandler');
        console.log(curtainState);
    
        if (curtainState == "CURTAIN_CLOSED") {
          IoTData.publish(OpenRollerParams, function(err, data) {
            if (err) {
              console.log(err);
            }
          });
          speechText = 'opening roller';
        } else if (curtainState == "CURTAIN_OPENED") {
          speechText = 'it is opened already';
        }
    
        return handlerInput.responseBuilder
          .speak(speechText)
          .reprompt(speechText)
          .getResponse();
        }
};

const CloseRollerIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'closeRoller';
    },
    async handle(handlerInput) {
        let curtainState;
        let speechText;
        await getShadowPromise(ShadowParams)
        .then((result) => curtainState = result.state.reported.curtainState);
        console.log('TurnOnIntentHandler');
        console.log(curtainState);
    
        if (curtainState == "CURTAIN_OPENED") {
          IoTData.publish(CloseRollerParams, function(err, data) {
            if (err) {
              console.log(err);
            }
          });
          speechText = 'closing roller';
        } else if (curtainState == "CURTAIN_CLOSED") {
          speechText = 'it is closed already';
        }
    
        return handlerInput.responseBuilder
          .speak(speechText)
          .reprompt(speechText)
          .getResponse();
        }
};

const EnableAutomaticModeIntentHandler = {
      canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
          && Alexa.getIntentName(handlerInput.requestEnvelope) === 'automaticModeOn';
      },
      async handle(handlerInput) {
        let detectLightState;
        let speechText;
        await getShadowPromise(ShadowParams)
        .then((result) => detectLightState = result.state.reported.detectLight);
        console.log('TurnOnIntentHandler');
        console.log(detectLightState);
    
        if (detectLightState == false) {
          IoTData.publish(detectLightTrueParams, function(err, data) {
            if (err) {
              console.log(err);
            }
          });
          speechText = 'setting automatic mode on';
        } else if (detectLightState == true) {
          speechText = 'automatic mode is already on';
        }
    
        return handlerInput.responseBuilder
          .speak(speechText)
          .reprompt(speechText)
          .getResponse();
        }
};

const DisableAutomaticModeIntentHandler = {
      canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
          && Alexa.getIntentName(handlerInput.requestEnvelope) === 'automaticModeOff';
      },
      async handle(handlerInput) {
        let detectLightState;
        let speechText;
        await getShadowPromise(ShadowParams)
        .then((result) => detectLightState = result.state.reported.detectLight);
        console.log('TurnOnIntentHandler');
        console.log(detectLightState);
    
        if (detectLightState == true) {
          IoTData.publish(detectLightFalseParams, function(err, data) {
            if (err) {
              console.log(err);
            }
          });
          speechText = 'setting automatic mode off';
        } else if (detectLightState == false) {
          speechText = 'automatic mode is already off';
        }
    
        return handlerInput.responseBuilder
          .speak(speechText)
          .reprompt(speechText)
          .getResponse();
        }
};

const HelpIntentHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
      && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.HelpIntent';
  },
  handle(handlerInput) {
    const speechText = 'you can open and close roller, also set automatic mode on and off';

    return handlerInput.responseBuilder
      .speak(speechText)
      .reprompt(speechText)
      .withSimpleCard('you can open and close roller, also set automatic mode on and off', speechText)
      .getResponse();
  }
};

const CancelAndStopIntentHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
      && (Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.CancelIntent'
        || Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.StopIntent');
  },
  handle(handlerInput) {
    const speechText = 'Goodbye my little pogchamp!';

    return handlerInput.responseBuilder
      .speak(speechText)
      .withSimpleCard('Goodbye my little pogchamp!', speechText)
      .withShouldEndSession(true)
      .getResponse();
  }
};

const ErrorHandler = {
  canHandle() {
    return true;
  },
  handle(handlerInput, error) {
    console.log(`Error handled: ${error.message}`);

    return handlerInput.responseBuilder
      .speak('Sorry, I don\'t understand your command. Please say it again.')
      .reprompt('Sorry, I don\'t understand your command. Please say it again.')
      .getResponse();
  }
};

const SessionEndedRequestHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'SessionEndedRequest';
  },
  handle(handlerInput) {
    // Any clean-up logic goes here.
    return handlerInput.responseBuilder.getResponse();
  }
};

exports.handler = Alexa.SkillBuilders.custom()
  .addRequestHandlers(
    LaunchRequestHandler,
    OpenRollerIntentHandler,
    CloseRollerIntentHandler,
    EnableAutomaticModeIntentHandler,
    DisableAutomaticModeIntentHandler,
    HelpIntentHandler,
    CancelAndStopIntentHandler,
    SessionEndedRequestHandler)
  .addErrorHandlers(ErrorHandler)
  .lambda();