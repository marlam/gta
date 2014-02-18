/*
 * Copyright (C) 2013, 2014
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "navigator.hpp"

#include "glvm.hpp"
using namespace glvm;


Navigator::Navigator() :
    _viewport(0, 0, -1, -1),
    _center(0.0f), _radius(0.0f), _bias(toQuat(0.0f, vec3(0.0f))),
    _pos(0.0f), _rot(0.0f, 0.0f, 0.0f, 0.0f),
    _state(state_inactive)
{
}

bool Navigator::check_pos(const ivec2& pos)
{
    return (pos.x >= _viewport[0] && pos.x < _viewport[0] + _viewport[2]
            && pos.y >= _viewport[1] && pos.y < _viewport[0] + _viewport[3]);
}

vec3 Navigator::ballmap(const ivec2& p)
{
    //assert(_viewport[2] > 0);
    //assert(_viewport[3] > 0);

    int x = p.x - _viewport[0];
    int y = p.y - _viewport[1];
    int w = max(2, _viewport[2]);
    int h = max(2, _viewport[3]);

    // bring v=(x,y) to [-1..1]^2
    vec2 v(x, h - 1 - y);
    v /= vec2(w - 1, h - 1);
    v -= vec2(0.5f, 0.5f);
    v *= 2.0f;

    float ll = dot(v, v);
    if (ll > 1.0f) {
        // outside ArcBall
        return vec3(v / sqrt(ll), 0.0f);
    } else {
        // inside ArcBall
        return vec3(v, sqrt(1.0f - ll));
    }
}

void Navigator::set_viewport(const ivec4& vp)
{
    _viewport = vp;
}

void Navigator::set_scene(const vec3& center, float radius,
            const vec3& eye, const vec3& up)
{
    _center = center;
    _radius = radius;
    vec3 axis = cross(normalize(eye), normalize(up));
    float angle = acos(dot(normalize(eye), vec3(0.0f, 0.0f, +1.0f)));
    _bias = toQuat(angle, axis);
    reset();
}

float Navigator::suggest_near() const
{
    return suggest_far() / 1000.0f;
}

float Navigator::suggest_far() const
{
    return _radius * 50.0f;
}

vec3 Navigator::get_viewer_pos() const
{
    return _bias * (_pos + _center);
}

quat Navigator::get_viewer_rot() const
{
    return _bias * _rot;
}

mat4 Navigator::get_viewmatrix() const
{
    return translate(toMat4(-get_viewer_rot()), -get_viewer_pos());
}

void Navigator::reset()
{
    _pos = vec3(0.0f, 0.0f, 2.5f * _radius);
    _rot = toQuat(0.0f, vec3(0.0f, 0.0f, -1.0));
    _trans_2d = vec2(0.0f);
    _scale_2d = 1.0f;
    _state = state_inactive;
}

void Navigator::start_rot(const glvm::ivec2& pos)
{
    _last_pos = pos;
    _last_ballpos = ballmap(pos);
    _state = state_rot;
}

void Navigator::rot(const glvm::ivec2& pos)
{
    if (_state == state_rot && check_pos(pos)) {
        vec3 ballpos = ballmap(pos);
        vec3 normal = cross(_last_ballpos, ballpos);
        if (length(normal) > 0.001f) {
            vec3 axis = _rot * normal;
            float angle = -acos(dot(_last_ballpos, ballpos));
            angle *= (length(_pos) - _radius) / _radius;
            quat rot = toQuat(angle, axis);
            _pos = rot * _pos;
            _rot = rot * _rot;
        }
        _last_ballpos = ballpos;
    }
}

void Navigator::start_shift(const glvm::ivec2& pos)
{
    _last_pos = pos;
    _last_dist = length(_pos) - _radius;
    _state = state_shift;
}

void Navigator::shift(const glvm::ivec2& pos)
{
    if (_state == state_shift) {
        vec3 up = _rot * vec3(0.0f, 1.0f, 0.0f);
        vec3 view = _rot * vec3(0.0f, 0.0f, -1.0f);
        vec3 left = cross(up, view);
        vec2 shift_per_pixel = vec2(0.1f) + vec2(_last_dist) / vec2(_viewport[2], _viewport[3]);
        shift_per_pixel /= 20.0f / _radius;
        vec2 offset = vec2(pos - _last_pos) * shift_per_pixel;
        _pos += offset.x * left;
        _pos += offset.y * up;
        _last_pos = pos;
        _last_dist = length(_pos) - _radius;
    }
}

void Navigator::start_zoom(const glvm::ivec2& pos)
{
    _last_pos = pos;
    _last_dist = length(_pos) - _radius;
    _state = state_zoom;
}

void Navigator::zoom(const glvm::ivec2& pos)
{
    if (_state == state_zoom) {
        float distchange_per_pixel = 0.1f + _last_dist / _viewport[3];
        distchange_per_pixel /= 20.0f / _radius;
        float offset = (pos.y - _last_pos.y) * distchange_per_pixel;
        _pos += offset * normalize(_rot * vec3(0.0f, 0.0f, -1.0f));
        _last_pos = pos;
        _last_dist = length(_pos) - _radius;
    }
}

void Navigator::zoom(float wheel_rot)
{
    float distchange_per_degree = 0.1f + _last_dist / _viewport[3];
    distchange_per_degree /= 5.0f / _radius;
    float offset = degrees(-wheel_rot) * distchange_per_degree;
    _pos += offset * normalize(_rot * vec3(0.0f, 0.0f, -1.0f));
}

vec2 Navigator::get_translation_2d() const
{
    return _trans_2d * _scale_2d;
}

float Navigator::get_scale_2d() const
{
    return _scale_2d;
}

mat4 Navigator::get_viewmatrix_2d() const
{
    return scale(translate(mat4(1.0f), vec3(get_translation_2d(), 0.0f)), vec3(get_scale_2d()));
}

void Navigator::start_shift_2d(const ivec2& pos)
{
    _last_pos = pos;
    _state = state_shift_2d;
}

void Navigator::shift_2d(const ivec2& pos)
{
    if (_state == state_shift_2d) {
        vec2 shift_per_pixel = vec2(2.0f / _scale_2d) / vec2(min(_viewport[2], _viewport[3]));
        vec2 offset = vec2(pos.x - _last_pos.x, _last_pos.y - pos.y) * shift_per_pixel;
        _trans_2d += offset;
        _last_pos = pos;
    }
}

void Navigator::start_zoom_2d(const ivec2& pos)
{
    _last_pos = pos;
    _state = state_zoom_2d;
}

void Navigator::zoom_2d(const ivec2& pos)
{
    if (_state == state_zoom_2d) {
        float scalechange_per_pixel = 0.1f + _scale_2d / _viewport[3];
        scalechange_per_pixel /= 20.0f;
        float relchange = clamp((pos.y - _last_pos.y) * scalechange_per_pixel, -0.5f, +0.5f);
        _scale_2d += _scale_2d * relchange;
        _scale_2d = clamp(_scale_2d, 1e-5f, 1e5f);
        _last_pos = pos;
    }
}

void Navigator::zoom_2d(float wheel_rot)
{
    float scalechange_per_degree = 0.1f + _scale_2d / _viewport[3];
    scalechange_per_degree /= 5.0f;
    float relchange = clamp(degrees(-wheel_rot) * scalechange_per_degree, -0.5f, +0.5f);
    _scale_2d += _scale_2d * relchange;
    _scale_2d = glvm::clamp(_scale_2d, 1e-5f, 1e5f);
}
