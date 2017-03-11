#pragma once
#include <map>
#include "oxygine-framework.h"

using namespace oxygine;

DECLARE_SMART(Camera, spCamera);
class Camera : public Actor
{
public:

	spActor _content;

	Camera()
	{
		addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &Camera::onEvent));
		addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &Camera::onEvent));
		addEventListener(TouchEvent::MOVE, CLOSURE(this, &Camera::onEvent));
		addEventListener(TouchEvent::WHEEL_DIR, CLOSURE(this, &Camera::onEvent));

		_transform.identity();
	}

	void setContent(spActor content)
	{
		_content = content;
		addChild(content);
	}

	struct touch
	{
		Vector2 current;
		Vector2 previous;
	};

	std::map<int, touch> _touches;

	void onEvent(Event* ev)
	{
		TouchEvent* te = safeCast<TouchEvent*>(ev);
		const Vector2& pos{ te->localPosition.x, te->localPosition.y };

		if (te->type == TouchEvent::TOUCH_DOWN)
		{
			touch& t = _touches[te->index];
			t.previous = t.current = pos;
		}

		if (te->type == TouchEvent::TOUCH_UP)
		{
			auto it = _touches.find(te->index);
			if (it != _touches.end())
				_touches.erase(it);

			if (_touches.empty() && _onCameraChange)
				_onCameraChange();
		}

		if (te->type == TouchEvent::WHEEL_DIR)
		{
			if (te->wheelDirection.y != 0.0f)
			{
				double scale = te->wheelDirection.y < 0 ? 0.95 : 1.05;

				_transform.translate(-VectorD3(pos.x, pos.y, 0));
				_transform.scale(VectorD3(scale, scale, 1));
				_transform.translate(VectorD3(pos.x, pos.y, 0));

				if (_onCameraChange)
					_onCameraChange();
			}
		}

		if (te->type == TouchEvent::MOVE && !_touches.empty())
		{
			touch& t = _touches[te->index];
			t.previous = t.current;

			t.current = pos;

			if (_touches.size() == 1)
			{
				Vector2 offset = t.current - t.previous;
				_transform.translate(VectorD3(offset.x, offset.y, 0));
			}
			else
			{
				touch* p1, *p2;
				for (auto& t : _touches)
				{
					if (t.first == te->index)
						p1 = &t.second;
					else
						p2 = &t.second;
				}

				Vector2 center = (p1->current + p2->current) / 2.0;
				Vector2 prevCenter = (p1->previous + p2->previous) / 2.0;

				double dist = p1->current.distance(p2->current);
				double prevDist = p1->previous.distance(p2->previous);
				double scale = dist / prevDist;


				Vector2 offset = center - prevCenter;

				p1->previous = p1->current;
				p2->previous = p2->current;

				_transform.translate(Vector3(offset.x, offset.y, 0));


				_transform.translate(-Vector3(center.x, center.y, 0));
				_transform.scale(Vector3(scale, scale, 1));
				_transform.translate(Vector3(center.x, center.y, 0));
			}
		}

		update();
	}

	void doUpdate(const UpdateState& us)
	{

	}

	void update()
	{
		Transform tr(_transform);
		_content->setTransform(tr);
	}

	float getCameraScale()
	{
		//return std::sqrt(_transform.m11*_transform.m11 + _transform.m12*_transform.m12);
		return _transform.m11;
	}

	oxygine::Vector2 getCameraOffset()
	{
		return oxygine::Vector2(_transform.m41, _transform.m42);
	}

	void reset()
	{
		_transform.identity();
	}

	Matrix _transform;

	std::function<void()> _onCameraChange;
};
