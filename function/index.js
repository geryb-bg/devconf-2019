"use strict";
const { google } = require("googleapis");
const config = require("./config.js");

exports.relayCloudIot = function(event, callback) {
    console.log(event.data ? Buffer.from(event.data, "base64").toString() : "nothing");
    
    if (event.data) {
        const record = JSON.parse(Buffer.from(event.data, "base64").toString());

        google.auth
            .getClient()
            .then(client => {
                google.options({
                    auth: client
                });
                
                const registryName = `projects/${config.projectId}/locations/${config.cloudRegion}/registries/${config.registryId}`;

                let binaryData, deviceName;
                if (record.light) {
                    deviceName = config.lightDeviceId;
                    binaryData = Buffer.from(record.light.toString()).toString("base64");
                } else if (record.temp) {
                    deviceName = config.fanDeviceId;
                    binaryData = Buffer.from(record.temp.toString()).toString("base64");
                } else {
                    console.log("Something went wrong! There was no light or fan value!");
                    return null;
                }

                const request = {
                    name: `${registryName}/devices/${deviceName}`,
                    versionToUpdate: 0,
                    binaryData: binaryData
                };
                console.log(`Send data to ${deviceName}`);
                return google
                    .cloudiot("v1")
                    .projects.locations.registries.devices.modifyCloudToDeviceConfig(request);
            })
            .then(result => {
                console.log(result);
            });
    }
};

//gcloud functions deploy relayCloudIot --runtime nodejs8 --trigger-topic=iot-topic
