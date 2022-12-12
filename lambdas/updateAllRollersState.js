// Include the Alexa SDK v2
const AWS = require('aws-sdk');
const IoTData = new AWS.IotData({endpoint: 'a13f6domxjrswp-ats.iot.us-east-1.amazonaws.com'});

let updateAllRollers = {
    topic: '$aws/things/cosin/shadow/update',
    payload: '',
    qos: 0
};


exports.handler = function(event, context) {
  /*
  "allRollersCurtainState": "CURTAIN_CLOSED",
  "rollerString": "[roller1]"*/
  let rollersString=JSON.parse(JSON.stringify(event))["rollerString"]
  let reportedCurtainState = JSON.parse(JSON.stringify(event))["allRollersCurtainState"]
  if(reportedCurtainState=="CURTAIN_CLOSED" || reportedCurtainState=="CURTAIN_OPENED")
  {

    let rollersArray = rollersString.split(",")
    console.log(rollersArray)

    let AnswerObject={"state":{"desired":{}}}
    rollersArray.forEach(rollerName => {
        AnswerObject["state"]["desired"]["rollers"]={...AnswerObject["state"]["desired"]["rollers"],[rollerName]:{curtainState:reportedCurtainState}}
    });
    updateAllRollers.payload = JSON.stringify(AnswerObject);
    console.log(updateAllRollers)
    IoTData.publish(updateAllRollers, function(err, data) {
                if (err) {
                  console.log(err);
                }
              });
  }
}