from comet_ml import Experiment
from Wrapper.layers import *
from Wrapper.wrappers import make_atari, wrap_deepmind, wrap_pytorch
import math, random
import gym
import numpy as np
import matplotlib.pyplot as plt
import torch
import torch.nn as nn
import torch.optim as optim
import torch.autograd as autograd
import torch.nn.functional as F
import csv


USE_CUDA = torch.cuda.is_available()
from dqn import QLearner, compute_td_loss, ReplayBuffer

env_id = "PongNoFrameskip-v4"
env = make_atari(env_id)
env = wrap_deepmind(env)
env = wrap_pytorch(env)

hyper_params = {
    "batch_size": 32,
    "gamma": 0.99,
    "buffer_size": 100000,
    "learn_rate": 0.00001,
    "epsilon_decay": 30000,
    "target_update": 50000,
    "file_number": 5,
    "model": 7
}

experiment = Experiment(api_key="XYZ",
                        project_name="pong-dqn", workspace="dhruvnandakumar")

experiment.log_parameters(hyper_params)

num_frames = 1000000
batch_size = 32
gamma = 0.99
record_idx = 10000

replay_initial = 10000
replay_buffer = ReplayBuffer(100000)
model = QLearner(env, num_frames, batch_size, gamma, replay_buffer)
model.load_state_dict(torch.load("model3.pth", map_location='cpu'))

target_model = QLearner(env, num_frames, batch_size, gamma, replay_buffer)
target_model.copy_from(model)

optimizer = optim.Adam(model.parameters(), lr=0.00001)
if USE_CUDA:
    model = model.cuda()
    target_model = target_model.cuda()
    print("Using cuda")

epsilon_start = 1.0
epsilon_final = 0.01
epsilon_decay = 30000
epsilon_by_frame = lambda frame_idx: epsilon_final + (epsilon_start - epsilon_final) * math.exp(
    -1. * frame_idx / epsilon_decay)

losses = []
all_rewards = []
episode_reward = 0

state = env.reset()



with experiment.train():

    for frame_idx in range(1, num_frames + 1):
        # print("Frame: " + str(frame_idx))

        epsilon = epsilon_by_frame(frame_idx)
        action = model.act(state, epsilon)

        next_state, reward, done, _ = env.step(action)
        replay_buffer.push(state, action, reward, next_state, done)

        state = next_state
        episode_reward += reward

        if done:
            state = env.reset()
            all_rewards.append((frame_idx, episode_reward))
            experiment.log_metric("episode_reward", episode_reward)
            with open("rewards5.txt", "w") as output:
                for row in all_rewards:
                    output.write(str(row[0]) + "," + str((row[1])) + "\n")

            with open("losses5.txt", "w") as output:
                for row in losses:
                    output.write(str(row[0]) + "," + str((row[1])) + "\n")

            episode_reward = 0

        if len(replay_buffer) > replay_initial:
            gamma1 = 0.5 + (0.99 - 0.5) * math.exp(
                -1. * frame_idx / 1000000)
            loss = compute_td_loss(model, target_model, batch_size, gamma1, replay_buffer)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            losses.append((frame_idx, loss.data.cpu().numpy()))

        if frame_idx % 10000 == 0 and len(replay_buffer) <= replay_initial:
            print('#Frame: %d, preparing replay buffer' % frame_idx)
            torch.save(model.state_dict(), "model7.pth")

        if frame_idx % 10000 == 0 and len(replay_buffer) > replay_initial:
            print('#Frame: %d, Loss: %f' % (frame_idx, np.mean(losses, 0)[1]))
            print('Last-10 average reward: %f' % np.mean(all_rewards[-10:], 0)[1])
            experiment.log_metric("mean_reward", np.mean(all_rewards))
            experiment.log_metric("last 10 reward", np.mean(all_rewards[-10:], 0)[1])
            experiment.log_metric("loss", loss.data.cpu().numpy())
            torch.save(model.state_dict(), "model7.pth")

        if frame_idx % 50000 == 0:
            target_model.copy_from(model)
