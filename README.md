# ESP32_Smart_AC
# 中文簡介

## Ｉ.Introduction
我們想做的是AI智慧冷氣控制儀，透過對環境的溫度、濕度以及使用者習慣的分析，減少使用者親自動手的次數，也能使室內處於舒適的溫度

## II. Idea and Inplementation

在ESP32上建立一個可以被手機訪問的webUI，使用者透過webUI對冷氣下指令，目前主要有以下幾種功能：

1.舒眠：使用者可以任意設定當下的冷氣溫度，而冷氣一定會在2~3小時內自動調整至“舒眠溫度”，預設的舒眠溫度是29度。

2.定時：設定冷氣以固定溫度運行多久，而在時間結束後，使用者可以選擇要自動關閉、進入舒眠模式或送風。

3.溫度感測：使用者設定目標溫度，冷氣會維持在目標溫度＋−1，可搭配定時使用。
在使用者設定的過程中，webUI中會跳出一些問題，是關於使用者的穿著、使用者是否處於運動狀態如此影響舒適度的要素，以及使用者的偏好設定、當前房間的濕度與溫度、冷氣的風速等已知數據傳送到香橙派進行統計與分析，隨著使用的次數越多，使用者越不需要去詳細設定參數，也能享受到舒適的冷氣。

## III. Conclusion

我們希望能創造一個更簡單的使用智慧家電的方法，不用特地購買昂貴的型號，也不用花時間去學習，讓AI在無形中幫助使用者。

# English Introduction

## Abstract

We aim to develop an **AI-based smart air-conditioning control system** that analyzes environmental temperature, humidity, and user habits to minimize manual adjustments and maintain a comfortable indoor environment.

## Motivation

We aim to explore the practicality of integrating AI into everyday life, and to develop our system with a **simple, human-centered approach**. By doing so, we hope to reduce people’s discomfort with advanced technologies and their concerns about high costs, ultimately making convenient technologies more **accessible to the general public**.

## Methodlogy

Use ESP32 for control AC and receive monitor signal, also for connecting **temperature and humidity level**. Orange Pi work for collecting data from esp32 and run a tiny AI model for **analyze and predict user’s habit**.

## Conclusion

We hope to create a **simpler way to use smart home appliances**, without requiring users to purchase expensive models or spend time learning how to use them, allowing **AI to assist users seamlessly in the background**.
















