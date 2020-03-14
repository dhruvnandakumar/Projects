package ecs160.visitor.mainapp;

import static org.junit.Assert.*;

import org.junit.BeforeClass;
import org.junit.Test;

import ecs160.visitor.astvisitors.SingletonCheckerVisitor;
import ecs160.visitor.astvisitors.StateCheckerVisitor;

public class TestHW2 {

    public static SingletonCheckerVisitor q1Grader;
    public static StateCheckerVisitor q2Grader;

    @BeforeClass
    public static void setUp()
    {
        /**
         * Create a static method that runs the AST set up code (similar to
         * what was shown in App.java), creates an visitor object of the described
         * type, runs .accept(...) on that visitor, and then returns it.  You may
         * implement the visitor however you wish (as well as any helper classes
         * or visitors), but the object should store enough information to
         * correctly grade all variants of HW1 (correct and incorrect) as specified
         * by the constraints in the HW2 Assignment.
         *
         * Each checker should define a grade<Part>() function that returns
         * your result for the parts of a)-d) for question 1, and a)-b) for question 2.
         *
         * This file is an example test case for 1 possible variant of the Homework 1
         * you are parsing (a correct one as specified by the question constraints).
         * You are responsible for changing the provided input files to check
         * that your code correctly handles incorrect variants, as our test cases
         * will also check these.
         *
         * NOTE: You may need to change these paths to where you put the Homework 1
         * Solution.
         */
        q1Grader = SingletonCheckerVisitor.setUpGrader("./src/main/java/ecs160/visitor/sampleFiles/PrintManager.java", "PrintManager");
        q2Grader = StateCheckerVisitor.setUpGrader("./src/main/java/hw1/LibraryBook.java",  "LibraryBook",
                "./src/main/java/hw1/LBState.java", "LBState");
        System.out.println(q1Grader);
        System.out.println(q2Grader);
    }

    @Test
    /**
     * Check if there is Private constructor
     */
    public void Question1a()
    {
        assertTrue(q1Grader.gradeA());
    }

    @Test
    /**
     * Check if there is a public static method that returns an instance of the class type
     */
    public void Question1b()
    {
        assertTrue(q1Grader.gradeB());
    }

    @Test
    /**
     * Check for a private static field of the right type.
     */
    public void Question1c()
    {
        assertTrue(q1Grader.gradeC());
    }

    @Test
    /**
     * Check that the constructor is called only once, from the public static method.
     * This call should be inside an if statement.
     * (Hint: Look at the ASTNode type ClassInstanceCreation).
     */
    public void Question1d()
    {
        assertTrue(q1Grader.gradeD());
    }

    @Test
    /**
     * Check that the CONTEXT has matching method names for each of methods in ABSTRACTSTATE.
     */
    public void Question2a()
    {
        assertTrue(q2Grader.gradeA());
    }

    @Test
    /**
     * Check that each of the CONTEXT methods calls the correct ABSTRACTSTATE method.
     * Return an integer that matches the number of methods correctly matched.
     */
    public void Question2b()
    {
        assertEquals(q2Grader.gradeB(), 4);
    }
}
