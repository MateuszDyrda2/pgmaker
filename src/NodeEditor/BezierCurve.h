#pragma once

#include <imgui.h>

class BezierCurve
{
    ImVec2 inputPos;
    ImVec2 outputPos;

    float inputBlockWidth, inputBlockHeight, outputBlockWidth, outputBlockHeight;

  public:
    BezierCurve(ImVec2 _inputPos, ImVec2 _outputPos, float _inputBlockWidth, float _inputBlockHeight, float _outputBlockWidth, float _outputBlockHeight):
        inputPos(_inputPos), outputPos(_outputPos), inputBlockWidth(inputBlockWidth), inputBlockHeight(inputBlockHeight), outputBlockWidth(_outputBlockWidth), outputBlockHeight(_outputBlockHeight) { }

    void setInputPos(ImVec2 pos)
    {
        inputPos = pos;
    }

    void setOutputPos(ImVec2 pos)
    {
        outputPos = pos;
    }

    void setPos(ImVec2 _inputPos, ImVec2 _outputPos)
    {
        inputPos  = _inputPos;
        outputPos = _outputPos;
    }

    void draw(ImDrawList* drawList)
    {
        if(inputPos.x > outputPos.x)
        {
            int sign = inputPos.y < outputPos.y ? -1 : 1;
            drawList->AddBezierQuadratic(inputPos, { (inputPos.x + inputBlockWidth / 2), inputPos.y }, { (inputPos.x + inputBlockWidth / 2), inputPos.y - (sign * (inputBlockHeight / 2)) }, 0xC0C0C0FF, 5.0f, 10);
            drawList->AddBezierQuadratic({ (inputPos.x + inputBlockWidth / 2), inputPos.y - (sign * (inputBlockHeight / 2)) }, { inputPos.x + (inputBlockWidth / 2), (inputPos.y + outputPos.y) / 2 }, { (inputPos.x + outputPos.x) / 2, (inputPos.y + outputPos.y) / 2 }, 0xC0C0C0FF, 5.0f, 10);
            drawList->AddBezierQuadratic({ (inputPos.x + outputPos.x) / 2, (inputPos.y + outputPos.y) / 2 }, { (outputPos.x + outputBlockWidth / 2), (inputPos.y + outputPos.y) / 2 }, { (outputPos.x + outputBlockWidth / 2), outputPos.y + (sign * (outputBlockHeight / 2)) }, 0xC0C0C0FF, 5.0f, 10);
            drawList->AddBezierQuadratic({ (outputPos.x + outputBlockWidth / 2), outputPos.y + (sign * (outputBlockHeight / 2)) }, { outputPos.x - (outputBlockWidth / 2), outputPos.y }, { outputPos.x, outputPos.y }, 0xC0C0C0FF, 5.0f, 10);
        }
        else if (inputPos.x == outputPos.x)
        {
            //do nothing
        }
        else
        {
            drawList->AddBezierQuadratic(inputPos, { (inputPos.x + outputPos.x) / 2, inputPos.y }, { (inputPos.x + outputPos.x) / 2, (inputPos.y + outputPos.y) / 2 }, 0xC0C0C0FF, 5.0f, 10);
            drawList->AddBezierQuadratic({ (inputPos.x + outputPos.x) / 2, (inputPos.y + outputPos.y) / 2 }, { (inputPos.x + outputPos.x) / 2, outputPos.y }, outputPos, 0xC0C0C0FF, 5.0f, 10);
        }
    }
};