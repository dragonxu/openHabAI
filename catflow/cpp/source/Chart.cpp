/* 
 * File:   Chart.h
 * Author: Joshua Johannson
 *
 */
#include <ApiMessage.h>
#include <util/TaskManager.h>
#include <Catflow.h>
#include "Chart.h"


Chart::Chart()
{
}

ApiRespond *Chart::processApi(ApiRequest request)
{
  ApiRespond *respond = ApiJsonObject::processApi(request);
  return respond;
}

void Chart::pushUpdate()
{

}


/**
 * -- ParameterChart -------------------------------------
 */
ParameterChart::ParameterChart()
{
}

ApiRespond *ParameterChart::processApi(ApiRequest request)
{
  ApiRespond *respond = Chart::processApi(request);

  // when chart params change
  if (request.what == "update")
  {
    if (updateFunc == nullptr && updateFuncRanged == nullptr)
      return new ApiRespondError("can't update chart data: updateFunction(used to generate output data from rangeInputs) is not set", request);

    // update chart data
    // @TODO if calc update is already in tasklist replace it
    if (!TaskManager::containsTaskOnceOnly(this))
    {
      TaskManager::addTaskOnceOnly([this, route = this->route, &data = this->data, outputs = this->outputs,
                                       fixedInputs = this->fixedInputs, rangeInputs = this->rangeInputs, &
                                       updateFunc = this->updateFunc]()
                                   {
                                     recalcData();
                                   }, this);
    }
    else
      warn("task already in! ");
  }

  return respond;
}


void ParameterChart::recalcData()
{
  map<int, vector<ChartDataPoint>> dataOut;

  // interpolate update function
  if (updateFunc != nullptr)
  {
    // for each rageInput
    function<void(vector<RangeParam>::const_iterator, vector<RangeParam>::const_iterator, map<int, float>)> createData =
        [&](vector<RangeParam>::const_iterator input,vector<RangeParam>::const_iterator inputsEnd, map<int, float> inputData)
        {
          if (input == inputsEnd)
          {
            //debug("create dataPoint at "+ Json{inputData}.dump());
            for (auto i: updateFunc(inputData, outputs))
              dataOut[i.first].push_back(ChartDataPoint(i.second, inputData));
            return;
          }

          // calc increase step for rangeInput
          float step;
          if (input->steps > 0)
            step = ((input->to - input->from) / input->steps);
          else
            step = 0;
          step = (step > 0) ? step : 0;
          //debug("progress input (perStep+="+to_string(step)+")"+ input->toJson().dump());

          // for each point of range
          for (float i = input->from; i <= input->to; i += step)
          {
            map<int, float> nInputData = inputData;
            nInputData.emplace(input->id, i);
            createData(input + 1, inputsEnd, nInputData);
            if (step == 0)
              break;
          }
        };

    // remove old data
    // data.clear();

    // generate new data
    map<int, float> inputDat;
    for (auto i: fixedInputs) // insert fixedInputs
      inputDat.emplace(i.id, i.value);
    createData(rangeInputs.begin(), rangeInputs.end(), inputDat); // generate data by rangeInputs
    //debug("will send to Catflow: " + route.get().toString() + "  =   " + to_string(data.size()) + Json(data).dump(2));
  }
  // range update function
  else if (updateFuncRanged != nullptr)
  {
    dataOut = updateFuncRanged(rangeInputs, fixedInputs, outputs);
  }

  // send data, update data in object
  sendToSubscribers(ApiRequest(route.get(), "updateData", dataOut));
  data = dataOut;
}



/**
 * -- SeriesChart -------------------------------------
 */
SeriesChart::SeriesChart()
{
  addAction("getAllData", [this](ApiRequest request) {
    return new ApiRespondOk(data, request);
  });
}

void SeriesChart::addDataPoint(int outputId, vector<float> inputValues, float outputValue)
{
  if (data.find(outputId) == data.end())
    data.emplace(outputId, vector<ChartDataPointSimple>());

  ChartDataPointSimple dataPoint = ChartDataPointSimple(outputValue, inputValues);
  data[outputId].push_back(dataPoint);

  // send data, update data in object
  if (route.is_initialized())
    sendToSubscribers(ApiRequest(route.get(), "addDataPoint", Json{
        {"outputId", outputId},
        {"dataPoint", dataPoint}
    }));
}

void SeriesChart::addDataPoint(string outputName, vector<float> inputValues, float outputValue)
{
  for (auto el : outputNames) {
    if (outputName == el.second)
      addDataPoint(el.first, inputValues, outputValue);
  }
}

void SeriesChart::clearData()
{
  for (auto el : data)
    el.second.clear();
  sendToSubscribers(ApiRequest(route.get(), "clearData"));
}
