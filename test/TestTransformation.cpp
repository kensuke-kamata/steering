#include <gtest/gtest.h>

#include "Transformation.h"

TEST(TestTransformation, NoTransform)
{
    glm::vec2 localPoint(10.0f, 20.0f);
    glm::vec2 position(0.0f, 0.0f);
    glm::vec2 heading(1.0f, 0.0f);
    glm::vec2 scale(1.0f, 1.0f);

    glm::vec2 worldPoint = steering::ToWorld(localPoint, position, heading, scale);

    EXPECT_NEAR(worldPoint.x, localPoint.x, 1e-5);
    EXPECT_NEAR(worldPoint.y, localPoint.y, 1e-5);
}

TEST(TestTransformation, TranslationOnly)
{
    glm::vec2 localPoint(10.0f, 20.0f);
    glm::vec2 position(100.0f, 200.0f);
    glm::vec2 heading(1.0f, 0.0f);
    glm::vec2 scale(1.0f, 1.0f);

    glm::vec2 worldPoint = steering::ToWorld(localPoint, position, heading, scale);

    EXPECT_NEAR(worldPoint.x, localPoint.x + position.x, 1e-5);
    EXPECT_NEAR(worldPoint.y, localPoint.y + position.y, 1e-5);
}

TEST(TestTransformation, ScaleOnly)
{
    glm::vec2 localPoint(10.0f, 20.0f);
    glm::vec2 position(0.0f, 0.0f);
    glm::vec2 heading(1.0f, 0.0f);
    glm::vec2 scale(2.0f, 2.0f);

    glm::vec2 worldPoint = steering::ToWorld(localPoint, position, heading, scale);

    EXPECT_NEAR(worldPoint.x, localPoint.x * scale.x, 1e-5);
    EXPECT_NEAR(worldPoint.y, localPoint.y * scale.y, 1e-5);
}

TEST(TestTransformation, RotationOnly)
{
    glm::vec2 localPoint(1.0f, 0.0f);
    glm::vec2 position(0.0f, 0.0f);
    glm::vec2 heading(0.0f, 1.0f); // 90 degrees counter-clockwise
    glm::vec2 scale(1.0f, 1.0f);

    glm::vec2 worldPoint = steering::ToWorld(localPoint, position, heading, scale);

    EXPECT_NEAR(worldPoint.x, 0.0f, 1e-5);
    EXPECT_NEAR(worldPoint.y, 1.0f, 1e-5);
}

TEST(TestTransformation, CombinedTransform)
{
    glm::vec2 localPoint(10.0f, 20.0f);
    glm::vec2 position(100.0f, 200.0f);
    glm::vec2 heading(0.0f, 1.0f); // 90 degrees counter-clockwise
    glm::vec2 scale(2.0f, 2.0f);

    glm::vec2 worldPoint = steering::ToWorld(localPoint, position, heading, scale);
    glm::vec2 expectedPoint = position + scale * glm::vec2(-localPoint.y, localPoint.x);

    EXPECT_NEAR(worldPoint.x, expectedPoint.x, 1e-5);
    EXPECT_NEAR(worldPoint.y, expectedPoint.y, 1e-5);
}
